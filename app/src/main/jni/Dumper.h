//
// Created by Ascarre on 12-04-2026.
//

#pragma once

#include "PropertyFlags.h"

void DumpLib(std::string out) {
    std::string TempPath = out + "/LibDump.dat";
    std::ofstream Dump(TempPath, std::ofstream::out | std::ofstream::binary);
    size_t LibSize = (Memory.ModuleEnd - Memory.ModuleBase);//Getting LibSize

    uintptr_t Start = Memory.ModuleBase;

    if (Dump.is_open()) {
        if (isFastDump) {
            auto *buffer = new uint8_t[LibSize];
            memset(buffer, '\0', LibSize);
            memcpy(buffer, reinterpret_cast<const void *>(Memory.ModuleBase), LibSize);
            Dump.write((char *) buffer, LibSize);
        } else {
            char *buffer = new char[1];
            while (LibSize != 0) {
                memcpy(buffer, reinterpret_cast<const void *>(Start++), 1);
                Dump.write(buffer, 1);
                --LibSize;
            }
        }
        Dump.close();
    }
    std::string RebuildOutput = out + "/" + Memory.TargetProcess;
    fix_so(TempPath.c_str(), RebuildOutput.c_str(), Memory.ModuleBase);//Fixing DumpedLib
    remove(TempPath.c_str());//Removing the Dumped .dat File
    exit(0);//Exit the Game
}

void DumpBlocks(uintptr_t FNamePool, uint32_t blockId, uint32_t blockSize, int& count, std::ofstream& Dump) {
    uintptr_t It = Read<uintptr_t>(FNamePool + Offsets::FNamePoolToBlocks + (blockId * Offsets::PointerSize));
    uintptr_t End = It + blockSize - Offsets::FNameEntryToString;
    uint32_t Block = blockId;
    uint16_t Offset = 0;
    while (It < End) {
        uintptr_t FNameEntry = It;
        int16_t FNameEntryHeader = Read<int16_t>(FNameEntry + Offsets::FNameEntryHeader);
        int StrLength = FNameEntryHeader >> Offsets::FNameEntryToLenBit;
        if (StrLength) {
            bool wide = FNameEntryHeader & 1;

            ///Unicode Dumping Not Supported
            if (StrLength > 0) {
                //String Length Limit
                if (StrLength < 250) {
                    uint32_t key = (Block << 16 | Offset);
                    uintptr_t StrPtr = FNameEntry + Offsets::FNameEntryToString;

                    std::string Names = wide ? WideStr::getString(StrPtr, StrLength) : ReadStringNew(StrPtr, StrLength);

                    if (Offsets::isXorDecrypt) {
                        Dump << (wide ? "Wide \n" : "") << "[" << count << "]: " << DecryptXorCypher(Names) << " | Offset - 0x" << key << " | Length - " << StrLength << "\n";
                    } else {
                        Dump << (wide ? "Wide \n" : "") << "[" << count << "]: " << Names << " | Offset - 0x" << key << " | Length - " << StrLength << "\n";
                    }
                    count++;
                }
            }
            else {
                StrLength = -StrLength;//Negative lengths are for Unicode Characters
            }

            //Next
            Offset += StrLength / Offsets::FNameStride;
            uint16_t bytes = Offsets::FNameEntryToString + StrLength * (wide ? sizeof(wchar_t) : sizeof(char));
            It += (bytes + Offsets::FNameStride - 1u) & ~(Offsets::FNameStride - 1u);
        }
        else {// Null-terminator entry found
            break;
        }
    }
}

void DumpStrings(std::string out) {
    int count = 0;
    std::ofstream Dump(out + "/FNameStrings.txt", std::ofstream::out);
    if (Dump.is_open()) {
        if (Offsets::isUE423) {
            uintptr_t FNamePool = (Memory.ModuleBase + Offsets::GNames) + Offsets::GNamesToFNamePool;

            uint32_t BlockSize = Offsets::FNameStride * 65536;
            uint32_t CurrentBlock = Read<uint32_t>(FNamePool + Offsets::FNamePoolToCurrentBlock);
            uint32_t CurrentByteCursor = Read<uint32_t>(FNamePool + Offsets::FNamePoolToCurrentByteCursor);

            for (uint32_t BlockIdx = 0; BlockIdx < CurrentBlock; ++BlockIdx) {
                DumpBlocks(FNamePool, BlockIdx, BlockSize, count, Dump);
            }

            DumpBlocks(FNamePool, CurrentBlock, CurrentByteCursor, count, Dump);
        } else {
            for (uint32_t i = 0; i < 170000; i++) {
                std::string s = GetNameFromFName(i);
                if (!s.empty()) {
                    if (Offsets::isXorDecrypt) {
                        Dump << "[" << count << "]: " << DecryptXorCypher(s) << "\n";
                    } else {
                        Dump << "[" << count << "]: " << s << "\n";
                    }
                    count++;
                }
            }
        }
        Dump.close();
    }
}

void DumpObjects(std::string out) {
    int Count = 0;
    std::ofstream Dump(out + "/Objects.txt", std::ofstream::out);
    if (Dump.is_open()) {
        int32_t ObjectsCount = GetObjectCount();
        if (ObjectsCount < 10 || ObjectsCount > 999999) {
            ObjectsCount = 300000;
        }
        for (int32_t i = 0; i < ObjectsCount; i++) {
            uintptr_t ObjectPointer = GetUObjectFromID(i);
            if (!UObject::isValid(ObjectPointer)) continue;
            Dump << "[0x" << std::hex << i <<
                 "]: Name: " << UObject::getName(ObjectPointer) <<
                 " | Class: " << UObject::getClassName(ObjectPointer) <<
                 " | ObjectPtr: 0x" << std::hex << ObjectPointer <<
                 " | ClassPtr: 0x" << std::hex << UObject::getClassPrivate(ObjectPointer) << "\n";
            Count++;
        }
        Dump.close();
    }
}

void DumpSDK(std::string out) {
    std::ofstream Dump(out + "/SDK.txt", std::ofstream::out);

    if (Dump.is_open()) {
        int32_t ObjectsCount = GetObjectCount();
        if (ObjectsCount < 10 || ObjectsCount > 999999) {
            ObjectsCount = 300000;
        }
        for (int32_t i = 0; i < ObjectsCount; i++) {
            uintptr_t ObjectPointer = GetUObjectFromID(i);
            if (!UObject::isValid(ObjectPointer)) continue;
            DumpStructures(Dump, UObject::getClassPrivate(ObjectPointer));
        }
        Dump.close();
    }
}

void DumpSDKW(std::string out) {
    std::ofstream Dump(out + "/SDKW.txt", std::ofstream::out);
    if (Dump.is_open()) {
        uintptr_t GWorld = Read<uintptr_t>(Memory.ModuleBase + Offsets::GWorld);
        if (!UObject::isValid(GWorld)) {
            return;
        }

        DumpStructures(Dump, UObject::getClassPrivate(GWorld));

        uintptr_t PersistentLevel = Read<uintptr_t>(GWorld + Offsets::UWorldToPersistentLevel);
        uintptr_t AActors = Read<uintptr_t>(PersistentLevel + Offsets::ULevelToAActors);
        int AActorsCount = Read<int>(PersistentLevel + Offsets::ULevelToAActorsCount);
        for (int i = 0; i < AActorsCount; i++) {
            uintptr_t Base = Read<uintptr_t>(AActors + (i * Offsets::PointerSize));
            if (UObject::isValid(Base)) {
                DumpStructures(Dump, UObject::getClassPrivate(Base));
            }
        }
        Dump.close();
    }
    std::cout << "[*] Dumped SDKW using GWorld...\n" << std::endl;
}

void DumpActors(std::string out) {
    int count = 0;
    std::cout << "\n[*] Dumping Actors from World..." << std::endl;
    std::ofstream Dump(out + "/ActorsDump.txt", std::ofstream::out);
    if (Dump.is_open()) {
        uintptr_t GWorld = Read<uintptr_t>(Memory.ModuleBase + Offsets::GWorld);
        if (!UObject::isValid(GWorld)) {
            std::cout << "[!] GWorld is Invalid..." << std::endl;
            return;
        }

        uintptr_t PersistentLevel = Read<uintptr_t>(GWorld + Offsets::UWorldToPersistentLevel);
        uintptr_t AActors = Read<uintptr_t>(PersistentLevel + Offsets::ULevelToAActors);
        int AActorsCount = Read<int>(PersistentLevel + Offsets::ULevelToAActorsCount);

        for (int i = 0; i < AActorsCount; i++) {
            uintptr_t Base = Read<uintptr_t>(AActors + (i * Offsets::PointerSize));
            if (UObject::isValid(Base)) {
                Dump << "[" << count << "]: Name - " << UObject::getName(Base) << ", Address - 0x" << std::hex << Base << "\n";
                count++;
            }
        }
        Dump.close();
    }
    std::cout << "[*] Dumped Actors from World...\n" << std::endl;
}

void DumpBones(std::string out) {
    std::ofstream Dump(out + "/BonesDump.txt", std::ofstream::out);
    if (Dump.is_open()) {
        uintptr_t GWorld = Read<uintptr_t>(Memory.ModuleBase + Offsets::GWorld);
        if (!UObject::isValid(GWorld)) return;

        uintptr_t OwningGameInstance = Read<uintptr_t>(GWorld + 0x330); // GameInstance* OwningGameInstance; - Class: World.Object
        uintptr_t LocalPlayers = Read<uintptr_t>(OwningGameInstance + 0x38);// LocalPlayer* [] LocalPlayers; - Class: GameInstance.Object
        uintptr_t LocalPlayer = Read<uintptr_t>(LocalPlayers + 0);
        uintptr_t PlayerController = Read<uintptr_t>(LocalPlayer + 0x30); // PlayerController* PlayerController; - Class: Player.Object
        uintptr_t AcknowledgedPawn = Read<uintptr_t>(PlayerController + 0x2c8); // Pawn* AcknowledgedPawn; - Class: PlayerController.Controller.Actor.Object

        uintptr_t Mesh = Read<uintptr_t>(AcknowledgedPawn + 0x2a8); // SkeletalMeshComponent* Mesh; - Class: Character.Pawn.Actor.Object

        uintptr_t SkeletalMesh = Read<uintptr_t>(Mesh + 0x4b0); // SkeletalMesh* SkeletalMesh; - Class: SkinnedMeshComponent.MeshComponent.PrimitiveComponent.SceneComponent.ActorComponent.Object

        uintptr_t Skeleton = Read<uintptr_t>(SkeletalMesh + 0x88); // Skeleton* Skeleton; - Class: SkeletalMesh.StreamableRenderAsset.Object

        uintptr_t Sockets = Read<uintptr_t>(Skeleton + 0x190); // SkeletalMeshSocket*[] Sockets; - Class: Skeleton.Object

        int32_t SocketCount = Read<int32_t>(Skeleton + 0x198); // As Sockets is a TArray it has the count 8 byes from the data so (Sockets + 0x8)

        Dump << SocketCount << " bones found in total" << "\n\n";

        for (int x = 0; x < SocketCount; x++) {
            uintptr_t Socket = Read<uintptr_t>(Sockets + (x * Offsets::PointerSize));

            uintptr_t SocketNameIndex = Read<uintptr_t>(Socket + 0x28); // FName SocketName; - Class: SkeletalMeshSocket.Object
            std::string SocketName = GetNameFromFName(SocketNameIndex);

            uintptr_t BoneNameIndex = Read<uintptr_t>(Socket + 0x30);// FName BoneName; - Class: SkeletalMeshSocket.Object
            std::string BoneName = GetNameFromFName(BoneNameIndex);

            if (!BoneName.empty() && BoneName != "None") {
                Dump << std::left << "[ID: " << x << "] " << std::setw(3) << "| Bone: " << std::setw(35) << BoneName << "| Socket: " << SocketName << "\n";
            }
        }
        Dump.close();
    }
}