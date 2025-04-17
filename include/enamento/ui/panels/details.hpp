#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "sdf/commons.hpp"
#include "utils/tribool.hpp"

namespace ImGui
{
    bool CheckBoxTristate(const char* label, tribool* v_tristate)
    {
        bool ret;
        if (*v_tristate == tribool::unknown)
        {
            ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, true);
            bool b = false;
            ret = ImGui::Checkbox(label, &b);
            if (ret)
                *v_tristate = 1;
            ImGui::PopItemFlag();
        }
        else
        {
            bool b = (*v_tristate );
            ret = ImGui::Checkbox(label, &b);
            if (ret)
                *v_tristate = (int)b;
        }
        return ret;
    }
};

namespace ui{

using field_t = sdf::field_t ;

void ShowValidationError(const char* fieldName)
{
    ImGui::TextColored(ImVec4(1,0,0,1), "Invalid value for %s", fieldName);
}

//TODO: it would be nice to have sliders for vectors as well, yet they only accept a single scalar min/max value which is not great.
void RenderFields(const field_t* fields, int fields_count, void* base_address) {
    ImGui::Indent(20);
    // For each field render its ImGui widget.
    for (int i = 0; i < fields_count; i++) {
        ImGui::PushID(i);
        const field_t& field = fields[i];
        // Get a pointer to the field's data.
        // Note: We assume that the field’s data is located at base_address + offset.
        char* dataPtr = reinterpret_cast<char*>(base_address) + field.offset;
        
        // Use a label for ImGui (e.g., "Field #3")
        char label[64];
        sprintf(label, "%s", field.name);
        
        // Optional: if there is a tooltip, then we will setup a hover tip on the widget.
        // Begin a group to allow the reset button to be on the same line.
        ImGui::BeginGroup();

        bool modified = false;
        bool valid = true;
        
        // If field has a default value defined, display a reset button
        if (field.defval && !field.readonly) {
            //TODO: better spacing, but he idea is good.
            ImGui::Unindent();
            if (ImGui::Button("R",{12,20})) {
                // Copy default value over the stored value
                memcpy(dataPtr, field.defval, field.length);
                modified = true;
            }
            ImGui::SameLine();
        }

        switch (field.type) {
        case field_t::type_float: {
            // Assume the stored value is a float
            float currentValue = *reinterpret_cast<float*>(dataPtr);
            // Optionally, set min/max if available.
            float minValue = (field.min ? *reinterpret_cast<float*>(field.min) : -FLT_MAX);
            float maxValue = (field.max ? *reinterpret_cast<float*>(field.max) : FLT_MAX);
            
            // Use InputFloat with defined min and max if desired.
            // Here, we check readonly – if readonly, we disable editing.
            if (field.readonly) {
                if (minValue>-100 && maxValue<100) ImGui::SliderFloat(label, &currentValue, minValue, maxValue, "%.3f", ImGuiInputTextFlags_ReadOnly);
                else ImGui::InputFloat(label, &currentValue, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_ReadOnly);
            } else {
                int ret_widget;
                if (minValue>-100 && maxValue<100) ret_widget = ImGui::SliderFloat(label, &currentValue, minValue, maxValue, "%.3f");
                else ret_widget = ImGui::InputFloat(label, &currentValue, 0.0f, 0.0f, "%.3f");
                if (ret_widget) {
                    // enforce limits if defined.
                    if (field.min || field.max) {
                        if (currentValue < minValue || currentValue > maxValue) {
                            valid = false;
                        }
                    }
                    // run validation if provided
                    if (field.validate && !field.validate(&currentValue))
                        valid = false;
                    
                    if (valid)
                    {
                        *reinterpret_cast<float*>(dataPtr) = currentValue;
                        modified = true;
                    }
                }
            }
            break;
        }
        case field_t::type_int: {
            // Assume the stored value is an int.
            int currentValue = *reinterpret_cast<int*>(dataPtr);
            int minValue = (field.min ? *reinterpret_cast<int*>(field.min) : INT_MIN);
            int maxValue = (field.max ? *reinterpret_cast<int*>(field.max) : INT_MAX);
            
            if (field.readonly) {
                if (minValue>-100 && maxValue<100) ImGui::SliderInt(label, &currentValue, minValue, maxValue, "%d", ImGuiInputTextFlags_ReadOnly);
                else ImGui::InputInt(label, &currentValue, 0, 0, ImGuiInputTextFlags_ReadOnly);
            } else {
                int ret_widget;
                if (minValue>-100 && maxValue<100) ret_widget = ImGui::SliderInt(label, &currentValue, minValue, maxValue, "%d");
                else ret_widget = ImGui::InputInt(label, &currentValue, 0,0);
                if (ret_widget) {
                    if (field.min || field.max) {
                        if (currentValue < minValue || currentValue > maxValue) {
                            valid = false;
                        }
                    }
                    if (field.validate && !field.validate(&currentValue))
                        valid = false;
                    
                    if (valid)
                    {
                        *reinterpret_cast<int*>(dataPtr) = currentValue;
                        modified = true;
                    }
                }
            }
            break;
        }
        case field_t::type_vec2: {
            // Assume the stored value is an array of two floats.
            float currentValue[2] = { reinterpret_cast<float*>(dataPtr)[0],
                                      reinterpret_cast<float*>(dataPtr)[1] };
            float minValue[2] = { -FLT_MAX, -FLT_MAX };
            float maxValue[2] = { FLT_MAX, FLT_MAX };
            if (field.min) {
                memcpy(minValue, field.min, sizeof(minValue));
            }
            if (field.max) {
                memcpy(maxValue, field.max, sizeof(maxValue));
            }
            if (field.readonly) {
                ImGui::InputFloat2(label, currentValue, "%.3f", ImGuiInputTextFlags_ReadOnly);
            } else {
                if (ImGui::InputFloat2(label, currentValue, "%.3f")) {
                    // enforce limits per component
                    for (int k = 0; k < 2; k++) {
                        if (currentValue[k] < minValue[k] || currentValue[k] > maxValue[k])
                        {
                            valid = false;
                            break;
                        }
                    }
                    if (field.validate && !field.validate(currentValue))
                        valid = false;
                    
                    if (valid) {
                        // Write the new values back.
                        memcpy(dataPtr, currentValue, sizeof(currentValue));
                        modified = true;
                    }
                }
            }
            break;
        }
        case field_t::type_vec3: {
            // Assume the stored value is an array of three floats.
            float currentValue[3] = { reinterpret_cast<float*>(dataPtr)[0],
                                      reinterpret_cast<float*>(dataPtr)[1],
                                      reinterpret_cast<float*>(dataPtr)[2] };
            float minValue[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
            float maxValue[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
            if (field.min) {
                memcpy(minValue, field.min, sizeof(minValue));
            }
            if (field.max) {
                memcpy(maxValue, field.max, sizeof(maxValue));
            }
            if (field.readonly) {
                ImGui::InputFloat3(label, currentValue, "%.3f", ImGuiInputTextFlags_ReadOnly);
            } else {
                if (ImGui::InputFloat3(label, currentValue, "%.3f")) {
                    for (int k = 0; k < 3; k++) {
                        if (currentValue[k] < minValue[k] || currentValue[k] > maxValue[k])
                        {
                            valid = false;
                            break;
                        }
                    }
                    if (field.validate && !field.validate(currentValue))
                        valid = false;
                    
                    if (valid) {
                        memcpy(dataPtr, currentValue, sizeof(currentValue));
                        modified = true;
                    }
                }
            }
            break;
        }
        case field_t::type_ivec2: {
            // Assume stored as an array of two ints.
            int currentValue[2] = { reinterpret_cast<int*>(dataPtr)[0],
                                    reinterpret_cast<int*>(dataPtr)[1] };
            int minValue[2] = { INT_MIN, INT_MIN };
            int maxValue[2] = { INT_MAX, INT_MAX };
            if (field.min) {
                memcpy(minValue, field.min, sizeof(minValue));
            }
            if (field.max) {
                memcpy(maxValue, field.max, sizeof(maxValue));
            }
            if (field.readonly) {
                ImGui::InputInt2(label, currentValue, ImGuiInputTextFlags_ReadOnly);
            } else {
                if (ImGui::InputInt2(label, currentValue)) {
                    for (int k = 0; k < 2; k++) {
                        if (currentValue[k] < minValue[k] || currentValue[k] > maxValue[k]) {
                            valid = false;
                            break;
                        }
                    }
                    if (field.validate && !field.validate(currentValue))
                        valid = false;
                    
                    if (valid) {
                        memcpy(dataPtr, currentValue, sizeof(currentValue));
                        modified = true;
                    }
                }
            }
            break;
        }
        case field_t::type_ivec3: {
            // Assume stored as an array of three ints.
            int currentValue[3] = { reinterpret_cast<int*>(dataPtr)[0],
                                    reinterpret_cast<int*>(dataPtr)[1],
                                    reinterpret_cast<int*>(dataPtr)[2] };
            int minValue[3] = { INT_MIN, INT_MIN, INT_MIN };
            int maxValue[3] = { INT_MAX, INT_MAX, INT_MAX };
            if (field.min) {
                memcpy(minValue, field.min, sizeof(minValue));
            }
            if (field.max) {
                memcpy(maxValue, field.max, sizeof(maxValue));
            }
            if (field.readonly) {
                ImGui::InputInt3(label, currentValue, ImGuiInputTextFlags_ReadOnly);
            } else {
                if (ImGui::InputInt3(label, currentValue)) {
                    for (int k = 0; k < 3; k++) {
                        if (currentValue[k] < minValue[k] || currentValue[k] > maxValue[k]) {
                            valid = false;
                            break;
                        }
                    }
                    if (field.validate && !field.validate(currentValue))
                        valid = false;
                    
                    if (valid) {
                        memcpy(dataPtr, currentValue, sizeof(currentValue));
                        modified = true;
                    }
                }
            }
            break;
        }
        case field_t::type_bool:
        {
            // Assume stored as a bool.
            bool currentValue = *reinterpret_cast<bool*>(dataPtr);
            if (field.readonly) {
                ImGui::Checkbox(label, &currentValue);
            } else {
                if (ImGui::Checkbox(label, &currentValue)) {
                    if (field.validate && !field.validate(&currentValue))
                        valid = false;
                    
                    if (valid) {
                        *reinterpret_cast<bool*>(dataPtr) = currentValue;
                        modified = true;
                    }
                }
            }
            break;
        }
        case field_t::type_tribool:
        {
            tribool currentValue = *reinterpret_cast<tribool*>(dataPtr);
            if (field.readonly) {
                ImGui::CheckBoxTristate(label, &currentValue);
            } else {
                if (ImGui::CheckBoxTristate(label, &currentValue)) {
                    if (field.validate && !field.validate(&currentValue))
                        valid = false;
                    
                    if (valid) {
                        *reinterpret_cast<bool*>(dataPtr) = currentValue;
                        modified = true;
                    }
                }
            }
            break;
        }
        case field_t::type_shared_buffer: {
            // Assume the stored value is an int.
            int currentValue = *reinterpret_cast<int*>(dataPtr);
            int minValue = 0;
            int maxValue = global_shared.capacity();
            
            if (field.readonly) {
                ImGui::InputInt(label, &currentValue, 0, 0, ImGuiInputTextFlags_ReadOnly);
            } else {
                int ret_widget = ImGui::InputInt(label, &currentValue, 0,0);
                if (ret_widget) {
                    if (field.min || field.max) {
                        if (currentValue < minValue || currentValue > maxValue) {
                            valid = false;
                        }
                    }
                    if (field.validate && !field.validate(&currentValue))
                        valid = false;
                    
                    if (valid)
                    {
                        *reinterpret_cast<int*>(dataPtr) = currentValue;
                        modified = true;
                    }
                }
            }
            break;
        }
        case field_t::type_cfg: {
            //TODO: use the original one?
            struct extras_t {
                // Bit-fields. Please note that for printing or numeric limit checking you might want to define constants.
                uint32_t uid  : 12;   // Object identity. Zero means "not assigned"
                uint32_t gid  : 9;    // Object group. Zero means default group
                uint32_t idx  : 10;   // Material index, zero means special NONE.
                uint32_t weak : 1;    // If true, don't use material for contributions unless the others are also weak.
            };
            // Assume that we want to treat this as a special case with an extras_t layout.
            // Get a pointer to the extras_t stored in our memory.
            extras_t* extras = reinterpret_cast<extras_t*>(dataPtr);
            // Copy the current value into temporary variables.
            int uid  = extras->uid;
            int gid  = extras->gid;
            int idx  = extras->idx;
            bool weak = extras->weak;

            // Create an ID scope so that repeated widgets are uniquely identified.
            ImGui::PushID(label);

            // Render a grouped layout for each field.

            // Render an input for each bitfield with limits derived from the number of bits.
            // uid: 12 bits (0..4095)
            if (!field.readonly && ImGui::InputInt("UID", &uid)) {
                if (uid < 0) uid = 0;
                if (uid > 4095) uid = 4095;
            }
            if(ImGui::IsItemHovered())ImGui::SetTooltip("Unique Index (0 to keep unnamed)");
            // gid: 9 bits (0..511)
            if (!field.readonly && ImGui::InputInt("GID", &gid)) {
                if (gid < 0) gid = 0;
                if (gid > 511) gid = 511;
            }
            if(ImGui::IsItemHovered())ImGui::SetTooltip("Group index (511 Sky)");
            // idx: 10 bits (0..1023)
            if (!field.readonly && ImGui::InputInt("IDX", &idx)) {
                if (idx < 0) idx = 0;
                if (idx > 1023) idx = 1023;
            }
            if(ImGui::IsItemHovered())ImGui::SetTooltip("Material index");
            // weak: boolean flag
            if (!field.readonly)
                ImGui::Checkbox("Weak", &weak);
            if(ImGui::IsItemHovered())ImGui::SetTooltip("Material index");


            // Repack the values into our extras_t bitfield if everything is valid.
            // Optionally, you can choose to validate before writing back.
            if (!field.readonly) {
                extras->uid  = uid;
                extras->gid  = gid;
                extras->idx  = idx;
                extras->weak = weak;
            }

            ImGui::PopID();
            break;
        }
        //TODO: Missing enum. Use the description field to add named entries for each valid value.
        default:
            ImGui::Text("Unknown type for field %s", label);
            break;
        } // switch

        // Show tooltip if provided
        if (field.desc) {
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", field.desc);
            }
        }

        // If validation failed, show an error message.
        if (!valid) {
            ShowValidationError(label);
        }

        ImGui::EndGroup();

        // Optionally add spacing between fields.
        ImGui::Spacing();
        ImGui::PopID();
    }
    ImGui::Unindent(20);
}


//TODO: it would be nice to have sliders for vectors as well, yet they only accept a single scalar min/max value which is not great.
void RenderCfgFields( void* base_address) {
    ImGui::Indent(20);
    // For each field render its ImGui widget.
    {
        //TODO: use the original one?
        struct extras_t {
            // Bit-fields. Please note that for printing or numeric limit checking you might want to define constants.
            uint32_t uid  : 12;   // Object identity. Zero means "not assigned"
            uint32_t gid  : 9;    // Object group. Zero means default group
            uint32_t idx  : 10;   // Material index, zero means special NONE.
            uint32_t weak : 1;    // If true, don't use material for contributions unless the others are also weak.
        };
        // Assume that we want to treat this as a special case with an extras_t layout.
        // Get a pointer to the extras_t stored in our memory.
        extras_t* extras = reinterpret_cast<extras_t*>(base_address);
        // Copy the current value into temporary variables.
        int uid  = extras->uid;
        int gid  = extras->gid;
        int idx  = extras->idx;
        bool weak = extras->weak;

        // Create an ID scope so that repeated widgets are uniquely identified.
        ImGui::PushID("cfg");

        // Render a grouped layout for each field.

        // Render an input for each bitfield with limits derived from the number of bits.
        // uid: 12 bits (0..4095)
        if (ImGui::InputInt("UID", &uid)) {
            if (uid < 0) uid = 0;
            if (uid > 4095) uid = 4095;
        }
        if(ImGui::IsItemHovered())ImGui::SetTooltip("Unique Index (0 to keep unnamed)");
        // gid: 9 bits (0..511)
        if (ImGui::InputInt("GID", &gid)) {
            if (gid < 0) gid = 0;
            if (gid > 511) gid = 511;
        }
        if(ImGui::IsItemHovered())ImGui::SetTooltip("Group index (511 Sky)");
        // idx: 10 bits (0..1023)
        if (ImGui::InputInt("IDX", &idx)) {
            if (idx < 0) idx = 0;
            if (idx > 1023) idx = 1023;
        }
        if(ImGui::IsItemHovered())ImGui::SetTooltip("Material index");
        // weak: boolean flag
        ImGui::Checkbox("Weak", &weak);
        if(ImGui::IsItemHovered())ImGui::SetTooltip("Material index");


        // Repack the values into our extras_t bitfield if everything is valid.
        // Optionally, you can choose to validate before writing back.

        extras->uid  = uid;
        extras->gid  = gid;
        extras->idx  = idx;
        extras->weak = weak;

        ImGui::PopID();
    }
    ImGui::Spacing();    
    ImGui::Unindent(20);
}

}
