#pragma once

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <omp.h>

template<size_t ITEMS=2048, size_t ENTRY=0>
struct shared_map{
    struct view_t{
        void* base = nullptr;
        size_t size = 0;
    };

    struct cview_t{
        const void* base = nullptr;
        size_t size = 0;
    };
    
    #pragma omp declare target
    static inline view_t shared_entries[ITEMS];
    #pragma omp end declare target

    static inline size_t last=0;

    static inline constexpr size_t capacity() {return ITEMS;}

    static view_t operator[](size_t i){
        //assert(i<ITEMS);
        if(i<ITEMS) return shared_entries[i];
        else return {nullptr,0};
    }

    //Reserve a buffer to the table
    static bool reserve(size_t i, size_t size){
        assert(omp_get_device_num()==omp_get_initial_device());
        assert(i<ITEMS);
        if(i<ITEMS){
            omp_free(shared_entries[i].base);
            shared_entries[i].base=omp_alloc(size);
            if(shared_entries[i].base==nullptr) return false;
            shared_entries[i].size=size;
            return true;
        }
        else return false;
    }

    //Copy a buffer to the table (and sync)
    static bool copy(size_t i, const cview_t& data){
        assert(omp_get_device_num()==omp_get_initial_device());
        assert(i<ITEMS);
        if(i<ITEMS){
            omp_free(shared_entries[i].base);
            shared_entries[i].base=omp_alloc(data.size);
            if(shared_entries[i].base==nullptr) return false;
            memcpy(shared_entries[i].base,data.base,data.size);
            shared_entries[i].size=data.size;
            return sync(i);
        }
        else return false;
    }

    //Assign a buffer to the table (and sync)
    static bool assign(size_t i, const view_t& data){
        assert(omp_get_device_num()==omp_get_initial_device());
        assert(i<ITEMS);
        if(i<ITEMS){
            omp_free(shared_entries[i].base);
            shared_entries[i]=data;
            return sync(i);
        }
        else return false;
    }

    //Force sync of an entry
    static bool sync(size_t i){
        assert(omp_get_device_num()==omp_get_initial_device());
        assert(i<ITEMS);
        if(i>=ITEMS)return false;
        auto devs = omp_get_num_devices();
        
        int ret = 0;

        auto data = shared_entries[i];
        auto original_dev = omp_get_device_num();

        #pragma omp parallel for reduction(+:ret)
        for(int dev=0;dev<devs;dev++){
            void* tmp_base=omp_target_alloc(data.size,dev);
            #pragma omp target device(dev)
            {
                omp_free(shared_entries[i].base);
                shared_entries[i].base=tmp_base;
                shared_entries[i].size=data.size;
            }
            ret+=omp_target_memcpy(tmp_base, data.base, data.size, 0, 0, dev, original_dev);
        }
        return ret==0;
    }

    //Force sync of a slice in an entry
    static bool sync(size_t i,size_t start, size_t end){
        assert(omp_get_device_num()==omp_get_initial_device());
        assert(i<ITEMS);
        if(i>=ITEMS)return false;
        assert(end<start);
        assert(end<=shared_entries[i].size);
        auto devs = omp_get_num_devices();
        
        int ret = 0;

        auto base = shared_entries[i].base;
        auto original_dev = omp_get_device_num();

        #pragma omp parallel for reduction(+:ret)
        for(int dev=0;dev<devs;dev++){
            void* tmp_base;
            #pragma omp target device(dev)
            {
                tmp_base=shared_entries[i].base;
            }
            ret+=omp_target_memcpy(tmp_base, base, end-start, start, start, dev, original_dev);
        }
        return ret==0;
    }
    
    //Clear all entries in the table
    static void clear(){
        for(auto& view: shared_entries){
            omp_free(view.base);
            view.base=nullptr;view.size=0;
        }

        auto devs = omp_get_num_devices();
        #pragma omp parallel for 
        for(int dev=0;dev<devs;dev++)
        for(auto& view: shared_entries){
            omp_free(view.base);
            view.base=nullptr;view.size=0;
        }
    }

    /**
     * @brief Search for the next free index 
     * 
     * @return size_t 
     */
    static int get_next(){
        for(uint i = 0;i<ITEMS;i++){
            if(shared_entries[(last+i)%ITEMS].base==nullptr){last=(last+i)%ITEMS;return last;}
        }
        return -1;  //Cannot find it.
    }
};

/**
 * A sharable resource, defined in the master device but made available to each device for offloading.
 */
struct shared{
    private:
        int* counter;
        void*  master;
        void** ptrs;
        size_t size;

    public:

    //TODO: check if this default initialization is ok
    shared(){
        if(omp_get_device_num()==omp_get_initial_device())
        {
            auto devs = omp_get_num_devices();
            ptrs=(void**)malloc(sizeof(void*)*devs);
            counter=(int*)malloc(sizeof(int));
            master=nullptr;
            for(int i=0;i<devs;i++){
                ptrs[i]=nullptr;
            }
            (*counter)=0;
        }
    };

    shared(size_t size):size(size){
        if(omp_get_device_num()==omp_get_initial_device())
        {
            auto devs = omp_get_num_devices();
            ptrs=(void**)malloc(sizeof(void*)*devs);
            counter=(int*)malloc(sizeof(int));
            master=malloc(size);
            for(int i=0;i<devs;i++){
                ptrs[i]=omp_target_alloc(size, i);
            }
            (*counter)=1;
        }
    }

    //TODO: provide also a move constructor and the relative = overloads.
    shared(const shared& src){
        if(omp_get_device_num()==omp_get_initial_device())
        {
            (*src.counter) ++;
        }

        counter=src.counter;
        master=src.master;
        ptrs=src.ptrs;
        size=src.size;
    }

    inline shared(void* src,size_t size):shared(size){
        provide(src);
    }

    int provide(void* src) const{
        if(omp_get_device_num()==omp_get_initial_device())
        {
            memcpy(master, src, size);
            return sync();
        }
        return 1;
    }

    void* get() const{
        if(omp_get_device_num()==omp_get_initial_device())
        {
            return master;
        }
        else{
            return ptrs[omp_get_device_num()];
        }
        return nullptr;
    }

    void* get(int i) const{
        if(i==omp_get_initial_device()) return master;

        assert(i< omp_get_num_devices());
        return ptrs[i];
    }

    int sync() const{
        if(omp_get_device_num()==omp_get_initial_device())
            {
            auto devs = omp_get_num_devices();
            int ret = 0;
            #pragma omp parallel for reduction(+:ret)
            for(int i=0;i<devs;i++){
                omp_target_memcpy(ptrs[i], master, size, 0, 0, i, omp_get_device_num());
            }
            return ret;
        }
        return 1;
    }

    int sync(size_t start, size_t end) const{
        if(omp_get_device_num()==omp_get_initial_device())
            {
            assert(end<start);
            assert(end<=size);
            auto devs = omp_get_num_devices();
            int ret = 0;
            #pragma omp parallel for reduction(+:ret)
            for(int i=0;i<devs;i++){
                omp_target_memcpy(ptrs[i], master, end-start, start, start, i, omp_get_device_num());
            }
            return ret;
        }
        return 1;
    }

    inline void clear() const{
        if(omp_get_device_num()==omp_get_initial_device())
        {
            auto devs = omp_get_num_devices();
            for(int i=0;i<devs;i++){
                omp_target_free(ptrs[i], i);
            }
            free(master);
            free(ptrs);
            free(counter);
        }
    }

    inline ~shared(){
        if(omp_get_device_num()==omp_get_initial_device())
        {
            (*counter)--;
            if((*counter)==0)clear();
        }
        //clear();
    }
};