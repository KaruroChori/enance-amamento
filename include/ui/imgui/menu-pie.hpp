#pragma once
#include <imgui.h>

bool BeginPiePopup(const char* pName, int iMouseButton=ImGuiMouseButton_Right);
void EndPiePopup();

bool PieMenuItem(const char* pName, bool bEnabled = true);
bool BeginPieMenu(const char* pName, bool bEnabled = true);
void EndPieMenu();
