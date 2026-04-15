//
// Created by Ascarre on 12-04-2026.
//

#pragma once

#include "FNames.h"

int32_t GetObjectCount() {
    if (Offsets::isUE423) {
        return Read<int32_t>((Memory.ModuleBase + Offsets::GUObjectArray) + Offsets::FUObjectArrayToTUObjectArray + Offsets::TUObjectArrayToNumElements);
    }
    else {
        if (Offsets::isDereferencing) {
            return Read<int32_t>(Read<uintptr_t>((Memory.ModuleBase + Offsets::GUObjectArray)) + Offsets::FUObjectArrayToTUObjectArray + Offsets::TUObjectArrayToNumElements);
        }
        else {
            return Read<int32_t>((Memory.ModuleBase + Offsets::GUObjectArray) + Offsets::FUObjectArrayToTUObjectArray + Offsets::TUObjectArrayToNumElements);
        }
    }
}

uintptr_t GetUObjectFromID(uint32_t index) {
    if (Offsets::isUE423) {
        uintptr_t TUObjectArray = Read<uintptr_t>(Memory.ModuleBase + Offsets::GUObjectArray + Offsets::FUObjectArrayToTUObjectArray);
        uintptr_t Chunk = Read<uintptr_t>(TUObjectArray + ((index / 0x10000) * Offsets::PointerSize));
        return Read<uintptr_t>(Chunk + Offsets::FUObjectItemPadd + ((index % 0x10000) * Offsets::FUObjectItemSize));
    }
    else {
        uintptr_t FUObjectArray;
        if (Offsets::isDereferencing) {
            FUObjectArray = Read<uintptr_t>(Memory.ModuleBase + Offsets::GUObjectArray);
        }
        else {
            FUObjectArray = Memory.ModuleBase + Offsets::GUObjectArray;
        }
        uintptr_t TUObjectArray = Read<uintptr_t>(FUObjectArray + Offsets::FUObjectArrayToTUObjectArray);
        return Read<uintptr_t>(TUObjectArray + (index * Offsets::FUObjectItemSize));
    }
}