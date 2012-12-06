#include <sys/shm.h>
#include <errno.h>
#include "lsi_ctl.h"

LsiCtl* lsi_ctl_create(const char* cfg_file)
{
    if (!cfg_file)
    {
        return NULL;
    }

    LsiCtl* lsi_ctl = (LsiCtl*)malloc(sizeof(LsiCtl));
    memset(lsi_ctl, 0, sizeof(lsi_ctl));

    TiXmlDocument* cfgdoc = XmlOperator::LoadXml(cfg_file);
    if (!cfgdoc)
    {
        return NULL;
    }

    // lsi head
    TiXmlHandle cfghandler = XmlOperator::GetChildElement(cfgdoc, "LsiCfg");
    TiXmlHandle shmkey = XmlOperator::GetChildElement(cfghandler, "ShmKey");
    lsi_ctl->m_head.m_shm_key = atoi(XmlOperator::GetText(shmkey));
    TiXmlHandle count = XmlOperator::GetChildElement(cfghandler, "ChannelCount");
    lsi_ctl->m_head.m_chan_count = atoi(XmlOperator::GetText(count)) * 2;
    TiXmlHandle version = XmlOperator::GetChildElement(cfghandler, "Version");
    lsi_ctl->m_head.m_version = atoi(XmlOperator::GetText(version));
    lsi_ctl->m_head.m_size += sizeof(lsi_ctl->m_head);

    // lsi channel
    for (int i = 0; i < lsi_ctl->m_head.m_chan_count / 2; i++)
    {
        TiXmlHandle chan = XmlOperator::GetChildElement(cfghandler, "Channel", i);
        TiXmlHandle addr_1 = XmlOperator::GetChildElement(chan, "Address", 0);
        lsi_ip_t lsi_addr_1 = lsi_addr_aton(XmlOperator::GetText(addr_1));
        TiXmlHandle addr_2 = XmlOperator::GetChildElement(chan, "Address", 1);
        lsi_ip_t lsi_addr_2 = lsi_addr_aton(XmlOperator::GetText(addr_2));
        TiXmlHandle size = XmlOperator::GetChildElement(chan, "Size");
        int chan_size = atoi(XmlOperator::GetText(size));

        lsi_ctl->m_chan[i * 2].m_from = lsi_addr_1;
        lsi_ctl->m_chan[i * 2].m_to = lsi_addr_2;
        lsi_ctl->m_chan[i * 2].m_size = ROUNDUP_POWOF_2(chan_size);

        lsi_ctl->m_chan[i * 2 + 1].m_from = lsi_addr_2;
        lsi_ctl->m_chan[i * 2 + 1].m_to = lsi_addr_1;
        lsi_ctl->m_chan[i * 2 + 1].m_size = ROUNDUP_POWOF_2(chan_size);

        lsi_ctl->m_head.m_size += (ROUNDUP_POWOF_2(chan_size) + sizeof(LsiChanHead)) * 2;
    }

    return lsi_ctl;
}

int lsi_ctl_init(LsiCtl* lsi_ctl)
{
    if (!lsi_ctl || lsi_ctl->m_head.m_size <= 0)
    {
        return -1;
    }

    // create share memory
    int sum = MemAlign(lsi_ctl->m_head.m_size, getpagesize());
    int shmid = shmget(lsi_ctl->m_head.m_shm_key, sum, 0666 | IPC_CREAT | IPC_EXCL);
    if (shmid < 0)
    {
        if (-1 == shmid && EEXIST == errno)
        {
            printf("LSI [%d] already exist fail\n", lsi_ctl->m_head.m_shm_key);
            return -1;
        }

        printf("LSI create share momory[%d] fail\n", lsi_ctl->m_head.m_shm_key);
        return -1;
    }

    // set init data
    char* pool = (char*)(shmat(shmid, 0, 0));
    memset(pool, 0, sum);
    memcpy(pool, &lsi_ctl->m_head, sizeof(lsi_ctl->m_head));
    char* chan = pool + sizeof(lsi_ctl->m_head);
    int last_chan_size = 0;
    for (int i = 0; i < lsi_ctl->m_head.m_chan_count; i++)
    {
        chan += last_chan_size;
        memcpy(chan, &lsi_ctl->m_chan[i], sizeof(LsiChanHead));
        last_chan_size = lsi_ctl->m_chan[i].m_size + sizeof(LsiChanHead);
    }
    shmdt(pool);

    return lsi_ctl_status(lsi_ctl);
}

int lsi_ctl_status(LsiCtl* lsi_ctl)
{
    if (!lsi_ctl || lsi_ctl->m_head.m_size <= 0)
    {
        return -1;
    }

    // get share memory
    int shmid = shmget(lsi_ctl->m_head.m_shm_key, 0, 0666);
    if (shmid < 0)
    {
        printf("LSI [%d] get fail\n", lsi_ctl->m_head.m_shm_key);
        return -1;
    }

    // check data
    char* pool = (char*)(shmat(shmid, 0, 0));
    LsiHead* load_head = (LsiHead*)pool;
    if (memcmp(load_head, &lsi_ctl->m_head, sizeof(LsiHead)))
    {
        printf("LSI [%d] head fail\n", lsi_ctl->m_head.m_shm_key);
        return -1;
    }
    char* chan = pool + sizeof(*load_head);
    int last_chan_size = 0;
    for (int i = 0; i < lsi_ctl->m_head.m_chan_count; i++)
    {
        chan += last_chan_size;
        LsiChanHead* chan_head = (LsiChanHead*)(chan);
        if (chan_head->m_from != lsi_ctl->m_chan[i].m_from
            || chan_head->m_to != lsi_ctl->m_chan[i].m_to
            || chan_head->m_size != lsi_ctl->m_chan[i].m_size)
        {
            printf("LSI[%s", lsi_addr_ntoa(chan_head->m_from));
            printf("-->%s] size=%d not equal to config file fail\n", lsi_addr_ntoa(chan_head->m_to), chan_head->m_size);
            return -1;
        }
        last_chan_size = chan_head->m_size + sizeof(LsiChanHead);
    }

    // print status
    printf(" LSI shmkey=%d, version=%d, channel count=%d\n",
           load_head->m_shm_key,
           load_head->m_version,
           load_head->m_chan_count);
    chan = pool + sizeof(*load_head);
    last_chan_size = 0;
    for (int i = 0; i < lsi_ctl->m_head.m_chan_count; i++)
    {
        chan += last_chan_size;
        LsiChanHead* chan_head = (LsiChanHead*)(chan);
        printf("  channel: [%s]", lsi_addr_ntoa(chan_head->m_from));
        printf("-->[%s], size=%d, read %u bytes, write %u bytes;\n",
               lsi_addr_ntoa(chan_head->m_to),
               chan_head->m_size,
               chan_head->m_read_bytes,
               chan_head->m_write_bytes);
        last_chan_size = chan_head->m_size + sizeof(LsiChanHead);
    }

    shmdt(pool);
    return 0;
}

int lsi_ctl_clean(LsiCtl* lsi_ctl)
{
    int shmid = shmget(lsi_ctl->m_head.m_shm_key, 0, IPC_CREAT);
    if (-1 == shmid)
    {
        printf("clean LSI [%d] fail: shm not exist\n", lsi_ctl->m_head.m_shm_key);
    }
    else if (0 == shmctl(shmid, IPC_RMID, NULL))
    {
        printf("clean LSI [%d] success\n", lsi_ctl->m_head.m_shm_key);
    }
    else
    {
        printf("clean LSI [%d] fail:5s\n", lsi_ctl->m_head.m_shm_key, strerror(errno));
    }
    return 0;
}

int lsi_ctl_destroy(LsiCtl* lsi_ctl)
{
    delete lsi_ctl;
    lsi_ctl = NULL;
    return 0;
}

