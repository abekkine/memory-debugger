#ifndef MEMORY_DEBUGGER_H
#define MEMORY_DEBUGGER_H

#include <stdint.h>

void memdbg_set_ports(int connect_port, int periodic_port);
void memdbg_set_address(const char * client_address);
void memdbg_init();
void memdbg_reset();
void memdbg_allocate(const char * label, void * address, unsigned int size);
void memdbg_release(const char * label, void * address, unsigned int size);
void memdbg_info(const char * message);
void memdbg_register_periodic(uint16_t id, const char * name);
void memdbg_update_periodic(uint16_t id, int typeId, void * value_ptr, int size);

#define MEMDBG_UPDATE_I8(id, value) memdbg_update_periodic(\
    id,\
    md_INT8,\
    &value,\
    1\
)

#define MEMDBG_UPDATE_I16(id, value) memdbg_update_periodic(\
    id,\
    md_INT16,\
    &value,\
    2\
)

#define MEMDBG_UPDATE_I32(id, value) memdbg_update_periodic(\
    id,\
    md_INT32,\
    &value,\
    4\
)

#define MEMDBG_UPDATE_I64(id, value) memdbg_update_periodic(\
    id,\
    md_INT64,\
    &value,\
    8\
)

#define MEMDBG_UPDATE_U8(id, value) memdbg_update_periodic(\
    id,\
    md_UINT8,\
    &value,\
    1\
)

#define MEMDBG_UPDATE_U16(id, value) memdbg_update_periodic(\
    id,\
    md_UINT16,\
    &value,\
    2\
)

#define MEMDBG_UPDATE_U32(id, value) memdbg_update_periodic(\
    id,\
    md_UINT32,\
    &value,\
    4\
)

#define MEMDBG_UPDATE_U64(id, value) memdbg_update_periodic(\
    id,\
    md_UINT64,\
    &value,\
    8\
)

#define MEMDBG_UPDATE_F(id, value) memdbg_update_periodic(\
    id,\
    md_FLOAT,\
    &value,\
    4\
)

#define MEMDBG_UPDATE_D(id, value) memdbg_update_periodic(\
    id,\
    md_DOUBLE,\
    &value,\
    8\
)

#define MEMDBG_UPDATE_S(id, value) memdbg_update_periodic(\
    id,\
    md_STRING,\
    value,\
    strlen(value)\
)

#define MEMDBG_CTOR(name) memdbg_allocate(name, this, sizeof(*this));
#define MEMDBG_DTOR(name) memdbg_release(name, this, sizeof(*this));

#endif // MEMORY_DEBUGGER_H
