#include "memory_debugger.h"

#include <windows.h>
#include <string.h>

// DEBUG
#include <stdio.h>
// END

#define MEMDBG_LABEL_SIZE 32
#define MEMDBG_DEFAULT_PORT 2401

static CRITICAL_SECTION memdbg_lock_;
static SOCKET memdbg_sock_ = -1;
static int memdbg_port_ = -1;
static struct sockaddr_in memdbg_remote_host_;

static int memdbg_ready = 0;

static unsigned short memdbg_sequence = 0;

#pragma pack(push, 1)
struct MemoryAllocationData {
    unsigned short sequence;
    char type;
    unsigned int address;
    unsigned int size;
    char label[MEMDBG_LABEL_SIZE];
} memdbg_alloc_data_;
#pragma pack(pop)

void memdbg_send();

void memdbg_reset() {
    if (memdbg_ready == 0) {
        memdbg_init(MEMDBG_DEFAULT_PORT);
    }
    EnterCriticalSection(&memdbg_lock_);
    memdbg_sequence = 0;
    memdbg_alloc_data_.type = 'c';
    memdbg_send();
    LeaveCriticalSection(&memdbg_lock_);
}

void memdbg_init(int port) {
    if (memdbg_ready == 0) {
// DEBUG
        printf("size of data block %d\n", sizeof(memdbg_alloc_data_));
// END

        memdbg_port_ = port;
        memset(&memdbg_remote_host_, 0, sizeof(memdbg_remote_host_));
        memdbg_sock_ = -1;
        InitializeCriticalSection(&memdbg_lock_);

        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return;

        memdbg_sock_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (memdbg_sock_ == INVALID_SOCKET) return;

        memdbg_remote_host_.sin_family = AF_INET;
        memdbg_remote_host_.sin_port = htons(port);
        memdbg_remote_host_.sin_addr.s_addr = inet_addr("127.0.0.1");
        memdbg_ready = 1;
    }
}

void memdbg_allocate(const char * label, void * address, unsigned int size) {
    if (memdbg_ready == 0) {
        memdbg_init(MEMDBG_DEFAULT_PORT);
    }
    EnterCriticalSection(&memdbg_lock_);
    memdbg_alloc_data_.type = 'a';
    memdbg_alloc_data_.address = (unsigned int) address;
    memdbg_alloc_data_.size = size;
    strncpy(memdbg_alloc_data_.label, label, MEMDBG_LABEL_SIZE);
    memdbg_send();
    LeaveCriticalSection(&memdbg_lock_);
}

void memdbg_release(const char * label, void * address, unsigned int size) {
    if (memdbg_ready == 0) {
        memdbg_init(MEMDBG_DEFAULT_PORT);
    }
    EnterCriticalSection(&memdbg_lock_);
    memdbg_alloc_data_.type = 'r';
    memdbg_alloc_data_.address = (unsigned int) address;
    memdbg_alloc_data_.size = size;
    strncpy(memdbg_alloc_data_.label, label, MEMDBG_LABEL_SIZE);
    memdbg_send();
    LeaveCriticalSection(&memdbg_lock_);
}

void memdbg_send() {
    memdbg_alloc_data_.sequence = memdbg_sequence;
    sendto(
        memdbg_sock_,
        (const char *)&memdbg_alloc_data_,
        sizeof(memdbg_alloc_data_),
        0,
        (SOCKADDR *)&memdbg_remote_host_,
        sizeof(memdbg_remote_host_)
    );
    ++memdbg_sequence;
    // DEBUG
    printf("Sequence is now %04x\n", memdbg_sequence);
}
