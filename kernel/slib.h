#pragma once

#include "types.h"
#include "elf.h"

#define MAX_SHARED_LIBRARIES 32
#define MAX_PATH_LENGTH 256

typedef struct {
    u8 state;
    char path[MAX_PATH_LENGTH];
    ELFObject elf;
    u32 num_users;
} SharedLibrary;

extern SharedLibrary loaded_shared_libs[MAX_SHARED_LIBRARIES];

typedef struct {
    SharedLibrary* slib;
    u32 offset;
} OpenSharedLibrary;

void init_shared_libs();
void find_and_load_shared_lib(const char* name);
