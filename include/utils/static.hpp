#pragma once

//Just some utilities to integrate static values in the final binary.

template<typename T, T V>
struct ForceStatic{static constexpr inline T value = V;};

template<auto V>
constexpr void* SVal = (void*)&ForceStatic<decltype(V),V>::value;

