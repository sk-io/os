#pragma once

#include "types.h"
#include "elf.h"
#include "config.h"

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
void init_shared_libs_for_task(u32 task_id, ELFObject* elf);
