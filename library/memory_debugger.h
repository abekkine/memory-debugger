#ifndef MEMORY_DEBUGGER_H
#define MEMORY_DEBUGGER_H

void memdbg_init(int port);
void memdbg_reset();
void memdbg_allocate(const char * label, void * address, unsigned int size);
void memdbg_release(const char * label, void * address, unsigned int size);

#endif // MEMORY_DEBUGGER_H
