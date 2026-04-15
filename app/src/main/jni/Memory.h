//
// Created by Ascarre on 12-04-2026.
//

#pragma once

#include "Offsets.h"

uintptr_t GetModuleBase(const char* LibraryName = Memory.TargetProcess) {
    char buffer[1024];
    FILE *fp = fopen("/proc/self/maps", "rt");
    if (fp == NULL) return 0;

    uintptr_t address = 0;
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strstr(buffer, LibraryName)) {
            address = (uintptr_t)strtoul(buffer, NULL, 16);
            break;
        }
    }
    fclose(fp);
    return address;
}

uintptr_t GetModuleEnd(const char* LibraryName = Memory.TargetProcess) {
    char buffer[1024];
    FILE *fp = fopen("/proc/self/maps", "rt");
    if (fp == NULL) return 0;

    uintptr_t endAddress = 0;
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strstr(buffer, LibraryName)) {
            uintptr_t start, end;
            if (sscanf(buffer, "%lx-%lx", &start, &end) == 2) {
                if (end > endAddress) {
                    endAddress = end;
                }
            }
        }
    }
    fclose(fp);
    return endAddress;
}

bool isLibraryLoaded(const char* LibraryName = Memory.TargetProcess) {
    char line[512] = {0};
    FILE *fp = fopen("/proc/self/maps", "rt");
    bool isLoaded = false;

    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, LibraryName)) {
                isLoaded = true;
                break;
            }
        }
        fclose(fp);
    }
    return isLoaded;
}

template <typename T>
T Read(uintptr_t address) {
    return *((T*)address);
}

template<typename T>
std::vector<T> ReadArray(uintptr_t address, size_t size) {
    std::vector<T> data(size);
    for (int i = 0; i < size; i++) {
        data[i] = Read<T>(address + i * sizeof(T));
    }
    return data;
}

std::string ReadString(uintptr_t address, size_t  size = 100) {
    std::vector<char> data(size, '\0');

    for (int i = 0; i < size; i++) {
        data[i] = Read<char>(address + i);
        if (data[i] == 0x0) {
            break;
        }
    }

    std::string name(data.data());
    name.shrink_to_fit();
    return name;
}

std::string ReadStringNew(uintptr_t address, size_t  size = 100) {
    std::string name(size, '\0');
    for (unsigned int i = 0; i < size; i++) {
        name[i] = Read<char>(address + i);
    }
    name.shrink_to_fit();
    return name;
}

std::string DecryptXorCypher(std::string Input, int size = 0) {
    if (size == 0) size = Input.size();
    std::string Output = Input;

    for (int i = 0; i < size; i++)
        Output[i] = Input[i] ^ size;

    return Output;
}