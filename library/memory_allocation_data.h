#ifndef MEMORY_ALLOCATION_DATA_H
#define MEMORY_ALLOCATION_DATA_H

#define MEMDBG_LABEL_SIZE 48
#define MEMDBG_INFO_SIZE 54

enum MemDbgTypeEnum {
    md_NONE = 0,
    md_ALLOCATION,
    md_INFO,
    md_PERIODIC,
};

enum MemDbgSubTypeEnum {
    // Allocation
    md_RESET = 10,
    md_ALLOCATE,
    md_RELEASE,
    // Info
    md_MESSAGE = 20,
    md_REGISTER,
    // Periodic
    md_INT8 = 30,
    md_INT16,
    md_INT32,
    md_INT64,
    md_UINT8,
    md_UINT16,
    md_UINT32,
    md_UINT64,
    md_FLOAT,
    md_DOUBLE,
    md_STRING,
};

#pragma pack(push, 1)
struct MemoryDataHeader {
    unsigned short  sequenceNo; // sequence number
    unsigned char   typeId;     // allocation / info / periodic
    unsigned char   subTypeId;  // vary with type
    unsigned int    threadId;   // source thread identifier
}; // 12 bytes total

struct MemoryAllocationData {
    struct MemoryDataHeader header;                     // 8
    unsigned int            address;                    // 4
    unsigned int            size;                       // 4
    char                    label[MEMDBG_LABEL_SIZE];   // 48
};

// send message OR send id/label for periodic
struct MemoryInfoData {
    struct MemoryDataHeader header;                     // 8
    unsigned short id;
    char                    message[MEMDBG_INFO_SIZE];    // 54
};

struct MemoryPeriodicData {
    struct MemoryDataHeader header; // 8
    unsigned short  id;             // 2
    unsigned char   value[8];       // 8
};
#pragma pack(pop)

#endif // MEMORY_ALLOCATION_DATA_H
