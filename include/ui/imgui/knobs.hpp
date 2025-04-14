#pragma once

/*
MIT License

Copyright (c) 2022 Simon Altschuler, 2025 Karurochari

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

//https://github.com/altschuler/imgui-knobs for the original implementation from which this is derived

#include <imgui.h>



namespace ImGuiKnobs {
    struct Enum {
        private: int data;
        public:

        constexpr Enum(int data):data(data){}
        constexpr operator int&(){return data;}
    };

    struct Flags : Enum {
        using Enum::Enum;

        enum{
            NoTitle = 1 << 0,
            NoInput = 1 << 1,
            ValueTooltip = 1 << 2,
            DragHorizontal = 1 << 3,
            DragVertical = 1 << 4,
            Logarithmic = 1 << 5,
            AlwaysClamp = 1 << 6
        };
    };

    struct Variant : Enum {
        using Enum::Enum;

        enum {
            Tick = 1 << 0,
            Dot = 1 << 1,
            Wiper = 1 << 2,
            WiperOnly = 1 << 3,
            WiperDot = 1 << 4,
            Stepped = 1 << 5,
            Space = 1 << 6,
        };
    };

    struct color_set {
        ImColor base;
        ImColor hovered;
        ImColor active;

        color_set(ImColor base, ImColor hovered, ImColor active)
            : base(base), hovered(hovered), active(active) {}

        color_set(ImColor color) {
            base = color;
            hovered = color;
            active = color;
        }
    };

    bool Knob(
            const char *label,
            float *p_value,
            float v_min,
            float v_max,
            float speed = 0,
            const char *format = "%.3f",
            Variant variant = Variant::Tick,
            float size = 0,
            Flags flags = 0,
            int steps = 10,
            float angle_min = -1,
            float angle_max = -1
            );

    bool KnobInt(
            const char *label,
            int *p_value,
            int v_min,
            int v_max,
            float speed = 0,
            const char *format = "%i",
            Variant variant = Variant::Tick,
            float size = 0,
            Flags flags = 0,
            int steps = 10,
            float angle_min = -1,
            float angle_max = -1
            );

}// namespace ImGuiKnobs