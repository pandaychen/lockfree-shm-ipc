#ifndef LSI_TOOL_H_
#define LSI_TOOL_H_

#include "lsi.h"

// lsi share momory head
typedef struct LsiHead
{
    int m_version;
    int m_size; // total size, all channels including
    int m_chan_count;
    int m_shm_key;
}LsiHead;

// lsi channel head
typedef struct LsiChanHead
{
    int m_size; // channel size(head not included)
    lsi_ip_t  m_from;
    lsi_ip_t  m_to;
    int start_idx;  // buffer read/write flags
    int end_idx;  // buffer read/write flags
    unsigned int read_bytes;    // statics info
    unsigned int write_bytes;   // statics info
}LsiChanHead;

// memory align with base
#define MemAlign(size, base)  ((size + base - 1) & (~(base -1)))

#endif // LSI_TOOL_H_

