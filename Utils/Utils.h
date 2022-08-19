#pragma once
#include <assert.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include "Types.h"

#define DEFINE_HANDLE(object) typedef struct object##_T *object;
#define ISPOT(x) ((x&(x-1))==0)
#define ISALIGNED(x, pot) ((x & (pot-1)) == 0)
#define ALIGN(x, pot) ((x+(pot-1))&(~(pot-1)))

#define ARRAYCOUNT(arr) (sizeof(arr)/sizeof(arr[0]))
#define TOSTRING(str) (#str)

namespace Phoenix
{
    static std::vector<byte_t> LoadFile(std::filesystem::path filePath)
    {
        std::vector<byte_t> fileData;
        // open the file
        std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

        // if open was successful
        if (file.is_open())
        {
            // find the length of the file
            int length = (int)file.tellg();

            // collect the file data
            fileData.resize(length);
            file.seekg(0, std::ios::beg);
            file.read(reinterpret_cast<char*>(fileData.data()), length);
            file.close();
        }

        return fileData;
    }
}
