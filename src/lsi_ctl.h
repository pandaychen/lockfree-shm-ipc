#ifndef LSI_CTL_H_
#define LSI_CTL_H_

#include "lsi_tool.h"
#include "xml/xml_wrapper.h"

//
// shmem memory organized like this:
// LsiHead <---> LsiChanHead | LsiChannel <---> LsiChanHead | LsiChannel ...
// each pair has two channels: A->B & B->A
//

typedef struct LsiCtl
{
    LsiHead m_head;
    LsiChanHead m_chan[MAX_LSI_CHAN_COUNT];
} LsiCtl;

LsiCtl* lsi_ctl_create(const char* cfg_file);

int lsi_ctl_init(LsiCtl* lsi_ctl);

int lsi_ctl_status(LsiCtl* lsi_ctl);

int lsi_ctl_clean(LsiCtl* lsi_ctl);

int lsi_ctl_destroy(LsiCtl* lsi_ctl);

#endif // LSI_CTL_H_

