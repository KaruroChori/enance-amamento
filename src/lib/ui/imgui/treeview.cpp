#include "ui/imgui/treeview.hpp"

std::string GetTruncatedLabel(const char* label, float maxWidth){
    const char* ellipsis = "...";
    float ellipsisWidth = ImGui::CalcTextSize(ellipsis).x;
    float fullWidth = ImGui::CalcTextSize(label).x;

    // If the full text fits within maxWidth, return it unmodified.
    if (fullWidth <= maxWidth)
        return std::string(label);

    // Otherwise, build a truncated string.
    std::string truncated;
    float currentWidth = 0.0f;

    // Process one character at a time
    for (const char* p = label; *p; ++p)
    {
        char c[2] = { *p, '\0' };
        float charWidth = ImGui::CalcTextSize(c).x;

        // Check if adding this character and the ellipsis will not exceed the max width.
        if ((currentWidth + charWidth + ellipsisWidth) > maxWidth)
            break;

        truncated.push_back(*p);
        currentWidth += charWidth;
    }

    // Append ellipsis.
    truncated.append(ellipsis);
    return truncated;
}

int TreeNodeWithToggle(const char* label, bool open, bool leaf, bool selected, bool* buttonStates)
{
    constexpr int BUTTONS_SIZE = 60;
    // Create a unique ID for the tree node
    ImGui::PushID(label);

    // Render the tree node header using TreeNodeEx, but don't advance a new line until after
    // drawing the button.
    // Here we use ImGuiTreeNodeFlags_SpanAvailWidth to take up the full width.
    auto trunclabel = GetTruncatedLabel(label, ImGui::GetContentRegionAvail().x - BUTTONS_SIZE - 25 );
    
    //ImGui::SetNextItemOpen(open);
    int nodeOpen = ImGui::TreeNodeEx(trunclabel.c_str(), 
        ImGuiTreeNodeFlags_SpanAvailWidth | 
        ImGuiTreeNodeFlags_AllowOverlap | 
        (open?ImGuiTreeNodeFlags_DefaultOpen:0) | 
        (leaf?ImGuiTreeNodeFlags_Leaf:0) |
        (selected?ImGuiTreeNodeFlags_Selected:0)
    );

    if (ImGui::IsItemHovered() && std::string(label)!=trunclabel)
    {
        ImGui::BeginTooltip();
        ImGui::Text("%s",label);
        ImGui::EndTooltip();
    }

    // Calculate available horizontal space for alignment.
    // This gives you the current cursor position (start of tree node content)
    float currX = ImGui::GetCursorPosX();
    // Total content region available width.
    float availW = ImGui::GetContentRegionAvail().x;
    
    // Define the button label and compute its size.
    const char* buttonLabel = "Action";

    // Calculate the X position for the right-align: start position plus available width minus button width.
    float buttonX = currX + availW - BUTTONS_SIZE;

    // Move the cursor to the computed X position on the same line.
    ImGui::SameLine(buttonX);
    
    // Render the button.
    if (ImGui::SmallButton("A"))nodeOpen+=2*1;
    
    ImGui::SameLine();

    if (ImGui::SmallButton("B"))nodeOpen+=2*2;

    ImGui::SameLine();

    if (ImGui::SmallButton("C"))nodeOpen+=2*3;

    ImGui::PopID();

    return nodeOpen;
}
