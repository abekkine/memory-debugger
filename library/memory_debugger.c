#include "memory_debugger.h"

#include "memory_allocation_data.h"

#include <windows.h>
#include <ws2tcpip.h>
#include <string.h>
#include <memory.h>

#define MEMDBG_DEFAULT_CONNECT_PORT 2401
#define MEMDBG_DEFAULT_PERIODIC_PORT 2501

static int memdbg_connect_port_ = MEMDBG_DEFAULT_CONNECT_PORT;

// Allocation / Release
static CRITICAL_SECTION memdbg_alloc_lock_;
static SOCKET memdbg_alloc_sock_ = -1;
static unsigned short memdbg_alloc_seq_ = 0;
static struct MemoryAllocationData memdbg_alloc_data_;

static CRITICAL_SECTION memdbg_release_lock_;
static SOCKET memdbg_release_sock_ = -1;
static unsigned short memdbg_release_seq_ = 0;
static struct MemoryAllocationData memdbg_release_data_;

// Info
static CRITICAL_SECTION memdbg_info_lock_;
static SOCKET memdbg_info_sock_ = -1;
static unsigned short memdbg_info_seq_ = 0;
static struct MemoryInfoData memdbg_info_data_;

// Periodics
static CRITICAL_SECTION memdbg_periodic_lock_;
static SOCKET memdbg_periodic_sock_ = -1;
static int memdbg_periodic_port_ = MEMDBG_DEFAULT_PERIODIC_PORT;
static unsigned short memdbg_periodic_seq_ = 0;
static struct MemoryPeriodicData memdbg_periodic_data_;

static struct sockaddr_in memdbg_remote_host_;

static int memdbg_ready = 0;

#define MEMDBG_CHECK_READY if (memdbg_ready == 0) { memdbg_init(); }

void memdbg_init();
void memdbg_send_alloc();
int memdbg_init_wsa();
int memdbg_connect(SOCKET * ptr_sock);
int memdbg_init_periodic();
void memdbg_send_release();
void memdbg_send_info();
void memdbg_send_periodic();

void memdbg_set_ports(int p_connect, int p_periodic) {
    if (p_connect != -1) {
        memdbg_connect_port_ = p_connect;
    }
    if (p_periodic != -1) {
        memdbg_periodic_port_ = p_periodic;
    }
}

void memdbg_reset() {

    MEMDBG_CHECK_READY;

    EnterCriticalSection(&memdbg_alloc_lock_);
    memdbg_alloc_seq_ = 0;
    memdbg_alloc_data_.header.typeId = md_ALLOCATION;
    memdbg_alloc_data_.header.subTypeId = md_RESET;
    memdbg_alloc_data_.header.threadId = 0;
    memdbg_send_alloc();
    LeaveCriticalSection(&memdbg_alloc_lock_);
}

void memdbg_init() {
    // return if already init'd.
    if (memdbg_ready != 0) {
        return;
    }

    // initialize critical sections
    InitializeCriticalSection(&memdbg_alloc_lock_);
    InitializeCriticalSection(&memdbg_release_lock_);
    InitializeCriticalSection(&memdbg_info_lock_);
    InitializeCriticalSection(&memdbg_periodic_lock_);

    // initialize winsock
    if (memdbg_init_wsa()) {
        return;
    }

    // initialize alloc, info, and periodic sockets
    if (memdbg_connect(&memdbg_alloc_sock_)) {
        return;
    }

    if (memdbg_connect(&memdbg_release_sock_)) {
        return;
    }

    if (memdbg_connect(&memdbg_info_sock_)) {
        return;
    }

    if (memdbg_init_periodic()) {
        return;
    }

    memdbg_ready = 1;
}

int memdbg_init_wsa() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        return 1;
    }

    return 0;
}

int memdbg_connect(SOCKET * ptr_sock) {
    memset(&memdbg_remote_host_, 0, sizeof(memdbg_remote_host_));
    *ptr_sock = -1;

    *ptr_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*ptr_sock == INVALID_SOCKET) {
        return 1;
    }

    memdbg_remote_host_.sin_family = AF_INET;
    memdbg_remote_host_.sin_port = htons(memdbg_connect_port_);
    memdbg_remote_host_.sin_addr.s_addr = inet_addr("127.0.0.1");

    int rv = connect(*ptr_sock, (SOCKADDR *)&memdbg_remote_host_, sizeof(memdbg_remote_host_));
    if (rv == SOCKET_ERROR) {
        return 1;
    }

    return 0;
}

int memdbg_init_periodic() {
    memset(&memdbg_remote_host_, 0, sizeof(memdbg_remote_host_));
    memdbg_periodic_port_ = -1;

    memdbg_periodic_sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (memdbg_periodic_sock_ == INVALID_SOCKET) {
        return 1;
    }

    memdbg_remote_host_.sin_family = AF_INET;
    memdbg_remote_host_.sin_port = htons(memdbg_periodic_port_);
    memdbg_remote_host_.sin_addr.s_addr = inet_addr("127.0.0.1");

    return 0;
}

void memdbg_allocate(const char * label, void * address, unsigned int size) {

    MEMDBG_CHECK_READY;

    EnterCriticalSection(&memdbg_alloc_lock_);
    memdbg_alloc_data_.header.typeId = md_ALLOCATION;
    memdbg_alloc_data_.header.subTypeId = md_ALLOCATE;
    memdbg_alloc_data_.header.threadId = GetCurrentThreadId();
    memdbg_alloc_data_.address = (unsigned int) address;
    memdbg_alloc_data_.size = size;
    strncpy(memdbg_alloc_data_.label, label, MEMDBG_LABEL_SIZE);
    memdbg_send_alloc();
    LeaveCriticalSection(&memdbg_alloc_lock_);
}

void memdbg_release(const char * label, void * address, unsigned int size) {

    MEMDBG_CHECK_READY;

    EnterCriticalSection(&memdbg_release_lock_);
    memdbg_release_data_.header.typeId = md_ALLOCATION;
    memdbg_release_data_.header.subTypeId = md_RELEASE;
    memdbg_release_data_.header.threadId = GetCurrentThreadId();
    memdbg_release_data_.address = (unsigned int) address;
    memdbg_release_data_.size = size;
    strncpy(memdbg_release_data_.label, label, MEMDBG_LABEL_SIZE);
    memdbg_send_release();
    LeaveCriticalSection(&memdbg_release_lock_);
}

void memdbg_info(const char * message) {

    MEMDBG_CHECK_READY;

    EnterCriticalSection(&memdbg_info_lock_);
    memdbg_info_data_.header.typeId = md_INFO;
    memdbg_info_data_.header.subTypeId = md_MESSAGE;
    memdbg_info_data_.header.threadId = GetCurrentThreadId();
    memdbg_info_data_.id = 0;
    strncpy(memdbg_info_data_.message, message, MEMDBG_INFO_SIZE);
    memdbg_send_info();
    LeaveCriticalSection(&memdbg_info_lock_);
}

void memdbg_register_periodic(unsigned short id, const char * name) {

    MEMDBG_CHECK_READY;

    EnterCriticalSection(&memdbg_info_lock_);
    memdbg_info_data_.header.typeId = md_INFO;
    memdbg_info_data_.header.subTypeId = md_REGISTER;
    memdbg_info_data_.header.threadId = GetCurrentThreadId();
    memdbg_info_data_.id = id;
    strncpy(memdbg_info_data_.message, name, MEMDBG_INFO_SIZE);
    memdbg_send_info();
    LeaveCriticalSection(&memdbg_info_lock_);
}

void memdbg_update_periodic(unsigned short id, int typeId, void * value_ptr, int size) {

    MEMDBG_CHECK_READY;

    EnterCriticalSection(&memdbg_periodic_lock_);
    memdbg_periodic_data_.header.typeId = md_PERIODIC;
    memdbg_periodic_data_.header.subTypeId = typeId;
    memdbg_periodic_data_.header.threadId = GetCurrentThreadId();
    memdbg_periodic_data_.id = id;
    memset(memdbg_periodic_data_.value, 0, 8);
    memcpy(memdbg_periodic_data_.value, value_ptr, size > 8 ? 8 : size);
    memdbg_send_periodic();
    LeaveCriticalSection(&memdbg_periodic_lock_);
}

void memdbg_send_alloc() {
    memdbg_alloc_data_.header.sequenceNo = memdbg_alloc_seq_;
    send(
        memdbg_alloc_sock_,
        (const char *)&memdbg_alloc_data_,
        sizeof(memdbg_alloc_data_),
        0
    );
    ++memdbg_alloc_seq_;
}

void memdbg_send_release() {
    memdbg_release_data_.header.sequenceNo = memdbg_release_seq_;
    send(
        memdbg_release_sock_,
        (const char *)&memdbg_release_data_,
        sizeof(memdbg_release_data_),
        0
    );
    ++memdbg_release_seq_;
}

void memdbg_send_info() {
    memdbg_info_data_.header.sequenceNo = memdbg_info_seq_;
    send(
        memdbg_info_sock_,
        (const char *)&memdbg_info_data_,
        sizeof(memdbg_info_data_),
        0
    );
    ++memdbg_info_seq_;
}

void memdbg_send_periodic() {
    memdbg_periodic_data_.header.sequenceNo = memdbg_periodic_seq_;
    sendto(
        memdbg_periodic_sock_,
        (const char *)&memdbg_periodic_data_,
        sizeof(memdbg_periodic_data_),
        0,
        (SOCKADDR *)&memdbg_remote_host_,
        sizeof(memdbg_remote_host_)
    );
    ++memdbg_periodic_seq_;
}
