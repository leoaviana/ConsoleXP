#pragma once

#include <vector>


struct PatchDetails {
    unsigned virtualAddress;
    const char* hexBytes;
};

PatchDetails s_patches[] = {
    {
        0x004DCCF0, // lua_ScanDllStart
        "B8" "00000000" // mov eax, 1
        "C3"            // ret
    },
    {
        0x004E5CB0, // ScanDllStart
        "B8" "01000000"
        "A3" "74B4B600"

        "68" "FA5C4E00"  // push Extensions (NOT ASCENSION, maybe another custom patch if needed)
        "E8" "1C683800"
        "83C4" "04"

        "68" "095D4E00"  // push Awesome
        "E8" "0F683800"
        "83C4" "04"

        "68" "1D5D4E00"  // push ConsoleXP
        "E8" "02683800"
        "83C4" "04"

        "55"
        "8BEC"

        "E8" "8710F2FF"  // call 00406D70

        "E9" "EA5AF2FF"  // jmp 0040B7D8

        "CCCCCCCCCCCCCCCCCCCCCCCC"

        "457874656E73696F6E732E646C6C00" // Extensions
        "417765736F6D65576F746C6B4C69622E646C6C00" //Awesome
        "436F6E736F6C6558502E646C6C00" //ConsoleXP

        "000000000000"
    },
    {
        0x0040B7D0, // StartAddress
        "E9" "DBA40D00" // jmp 0x004E5CB0
        "909090" // nop (3 times)
    }
};