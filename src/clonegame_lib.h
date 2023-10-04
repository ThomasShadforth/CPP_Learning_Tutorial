#pragma once

#include <stdio.h>

//Included in order to get malloc
#include <stdlib.h>

//Included to get memset
#include <string.h>

//Included to get timestamps
#include <sys/stat.h>

//Defines
#ifdef _WIN32
#define DEBUG_BREAK() __debugbreak()
//#elif __linux__
//#define DEBUG_BREAK() __builtin_debugtrap()
//#elif __APPLE__
//#define DEBUG_BREAK() __builtin_trap()
#endif

//LOGGING
enum TextColor{
    TEXT_COLOR_BLACK,
    TEXT_COLOR_RED,
    TEXT_COLOR_GREEN,
    TEXT_COLOR_YELLOW,
    TEXT_COLOR_BLUE,
    TEXT_COLOR_MAGENTA,
    TEXT_COLOR_CYAN,
    TEXT_COLOR_WHITE,
    TEXT_COLOR_BRIGHT_BLACK,
    TEXT_COLOR_BRIGHT_RED,
    TEXT_COLOR_BRIGHT_GREEN,
    TEXT_COLOR_BRIGHT_YELLOW,
    TEXT_COLOR_BRIGHT_BLUE,
    TEXT_COLOR_BRIGHT_MAGENTA,
    TEXT_COLOR_BRIGHT_CYAN,
    TEXT_COLOR_BRIGHT_WHITE,
    TEXT_COLOR_COUNT
};


template <typename ...Args>
void _log(char* prefix, char* msg, TextColor textColour, Args... args){
    
    static char* textColourTable[TEXT_COLOR_COUNT] = {
        "\x1b[30m",
        "\x1b[31m",
        "\x1b[32m",
        "\x1b[33m",
        "\x1b[34m",
        "\x1b[35m",
        "\x1b[36m",
        "\x1b[37m",
        "\x1b[90m",
        "\x1b[91m",
        "\x1b[92m",
        "\x1b[93m",
        "\x1b[94m",
        "\x1b[95m",
        "\x1b[96m",
        "\x1b[97m"
    };
    
    char formatBuffer[8192] = {};
    sprintf(formatBuffer, "%s %s %s \033[0m", textColourTable[textColour], prefix, msg);

    char textBuffer[8192] = {};
    sprintf(textBuffer, formatBuffer, args...);

    puts(textBuffer);
}

#define SM_TRACE(msg, ...) _log("TRACE: ", msg, TEXT_COLOR_GREEN, ##__VA_ARGS__);
#define SM_WARN(msg, ...) _log("WARNING: ", msg, TEXT_COLOR_YELLOW, ##__VA_ARGS__);
#define SM_ERROR(msg, ...) _log("ERROR: ", msg, TEXT_COLOR_RED, ##__VA_ARGS__);

#define SM_ASSERT(x, msg, ...)                                     \
{                                                                  \
    if(!(x)){                                                      \
        SM_ERROR(msg, ##__VA_ARGS__);                              \
        DEBUG_BREAK();                                             \
    }                                                              \
}

//Bump Allocator
struct BumpAllocator{
    size_t capacity;
    size_t used;
    char* memory;
};

BumpAllocator make_bump_allocator(size_t size)
{
    BumpAllocator ba = {};

    ba.memory = (char*)malloc(size);

    if(ba.memory){
        ba.capacity = size;
        memset(ba.memory, 0, size); //Set memory to 0
    } else{
        SM_ASSERT(false, "Failed to allocate ")
    }

    return ba;

}

char* bump_alloc(BumpAllocator* bumpAllocator, size_t size){
    char* result = nullptr;

    size_t alignedSize = (size + 7) & ~ 7; //Makes sure the first 4 bits are set to 0
    if(bumpAllocator->used + alignedSize < bumpAllocator->capacity){
        result = bumpAllocator->memory + bumpAllocator->used;
        bumpAllocator->used += alignedSize;
    } else{
        SM_ASSERT(false, "BumpAllocator is full!");
    }

    return result;
}

//File I/O

long long get_timestamp(char* file){
    struct stat file_stat = {};
    stat(file, &file_stat);
    return file_stat.st_mtime;
}

bool file_exists(char* filePath){
    SM_ASSERT(filePath, "No filepath supplied!");

    auto file = fopen(filePath, "rb");
    if(!file){
        return false;
    }

    fclose(file);

    return true;
}

long get_file_size(char* filePath){
    SM_ASSERT(filePath, "No filepath supplied!");

    long fileSize = 0;
    auto file = fopen(filePath, "rb");

    if(!file){
        SM_ERROR("Failed opening File: %s", filePath);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    fclose(file);

    return fileSize;
}

char* read_file(char* filePath, int* fileSize, char* buffer){
    SM_ASSERT(filePath, "No filepath supplied!");
    SM_ASSERT(fileSize, "No filesize supplied!");

    *fileSize = 0;
    auto file = fopen(filePath, "rb");

    if(!file){
        SM_ERROR("Failed opening file: %s", filePath);
        return nullptr;
    }

    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    memset(buffer, 0, *fileSize + 1);
    fread(buffer, sizeof(char), *fileSize, file);

    fclose(file);

    return buffer;
}

char* read_file(char* filePath, int* fileSize, BumpAllocator* bumpAllocator){
    char* file = nullptr;
    long fileSize2 = get_file_size(filePath);

    if(fileSize2){
        char* buffer = bump_alloc(bumpAllocator, fileSize2 + 1);

        file = read_file(filePath, fileSize, buffer);
    }

    return file;
}

void write_file(char* filePath, char* buffer, int size){
    SM_ASSERT(filePath, "No filepath supplied!");
    SM_ASSERT(filePath, "No buffer supplied!");

    auto file = fopen(filePath, "wb");
    if(!file){
        SM_ERROR("Failed opening file: %s", filePath);
        return;
    }

    fwrite(buffer, sizeof(char), size, file);
    fclose(file);
}

bool copy_file(char* fileName, char* outputName, char* buffer){
    int fileSize = 0;
    char* data = read_file(fileName, &fileSize, buffer);

    auto outputFile = fopen(outputName, "wb");
    if(!outputFile){
        SM_ERROR("Failed opening file: %s", outputName);
        return false;
    }

    int result = fwrite(data, sizeof(char), fileSize, outputFile);
    if(!result){
        SM_ERROR("Failed opening File: %s", outputName);
        return false;
    }

    fclose(outputFile);

    return true;
}

bool copy_file(char* fileName, char* outputName, BumpAllocator* bumpAllocator){
    char* file = 0;
    long fileSize2 = get_file_size(fileName);

    if(fileSize2){
        char* buffer = bump_alloc(bumpAllocator, fileSize2 + 1);

        return copy_file(fileName, outputName, buffer);
    }

    return false;
}