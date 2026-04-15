//
// Created by Ascarre on 12-04-2026.
//

#pragma once

#include "JavaNative.h"

void DumperThread() {
    while (!isLibraryLoaded()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    Memory.ModuleBase = GetModuleBase();
    Memory.ModuleEnd = GetModuleEnd();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    while (Menu.isRunning) {
        if (isStringsDump && !isStringDumped) {
            DumpStrings(Menu.DumpLocation);
            isStringDumped = true;
        }

        if (isObjectsDump && !isObjectsDumped) {
            DumpObjects(Menu.DumpLocation);
            isObjectsDumped = true;
        }

        if (isSDKUDump && !isSDKUDumped) {
            DumpSDK(Menu.DumpLocation);
            isSDKUDumped = true;
        }

        if (isSDKWDump && !isSDKWDumped) {
            DumpSDKW(Menu.DumpLocation);
            isSDKWDumped = true;
        }

        if (isActorsDump && !isActorsDumped) {
            DumpActors(Menu.DumpLocation);
            isActorsDumped = true;
        }

        if (isBoneDump && !isBoneDumped) {
            DumpBones(Menu.DumpLocation);
            isBoneDumped = true;
        }
    }
}