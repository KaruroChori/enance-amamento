#include <fstream>
#include <vs-xml/builder.hpp>

#if __has_include("entries.autogen.hpp")
    #include "entries.autogen.hpp"
    //TODO: Check for library version to match the one of the entries. Just to ensure that when releasing this index gets updated first.
#else 
    #error "You will have to run `helper.ts` before compiling this application"
#endif

int main(){
    std::ofstream schema_file("./dist/root-website/schema.xml");
    xml::Builder schema_builder;

    //TODO: build in here

    auto schema_xml = *schema_builder.close();
    schema_xml.print(schema_file,{});

    schema_file.close();
    return 0;
}