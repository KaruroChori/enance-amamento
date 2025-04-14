#pragma once
#include <imgui.h>
#include <string>

std::string GetTruncatedLabel(const char* label, float maxWidth);
int TreeNodeWithToggle(const char* label, bool open, bool leaf, bool selected, bool* buttonStates=nullptr);
