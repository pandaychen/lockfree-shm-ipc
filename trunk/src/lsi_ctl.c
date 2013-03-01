#include <sys/shm.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include "lsi_ctl.h"

LsiCtl* lsi_ctl_create(const char* cfg_file)
{
    if (!cfg_file)
    {
        return NULL;
    }

    LsiCtl* lsi_ctl = (LsiCtl*)malloc(sizeof(LsiCtl));
    memset(lsi_ctl, 0, sizeof(lsi_ctl));

    mxml_node_t* cfgdoc = xml_load_file(cfg_file);
    if (!cfgdoc)
    {
        return NULL;
    }

    // lsi head
    mxml_node_t* cfghandler = xml_find_child_element(cfgdoc, cfgdoc, "LsiCfg");

    mxml_node_t* shmkey = xml_find_child_element(cfghandler, cfgdoc, "ShmKey");
    assert(shmkey);
    lsi_ctl->m_head.m_shm_key = atoi(xml_element_get_text(shmkey));

    mxml_node_t* count = xml_find_child_element(cfghandler, cfgdoc, "ChannelCount");
    assert(shmkey);
    lsi_ctl->m_head.m_chan_count = atoi(xml_element_get_text(count)) * 2;

    mxml_node_t* version = xml_find_child_element(cfghandler, cfgdoc, "Version");
    lsi_ctl->m_head.m_version = atoi(xml_element_get_text(version));
    lsi_ctl->m_head.m_size += sizeof(lsi_ctl->m_head);

    // lsi channel
    mxml_node_t* chan, *addr_1, *addr_2, *size;
    int i = 0;
    chan = version;
    while (1)
    {
        chan = xml_find_sibling_element(chan, cfgdoc, "Channel");
        if (!chan)
        {
            break;
        }
        addr_1 = xml_find_child_element(chan, cfgdoc, "Address");
        assert(addr_1);
        lsi_ip_t lsi_addr_1 = lsi_addr_aton(xml_element_get_text(addr_1));
        addr_2 = xml_find_sibling_element(addr_1, cfgdoc, "Address");
        assert(addr_2);
        lsi_ip_t lsi_addr_2 = lsi_addr_aton(xml_element_get_text(addr_2));
        size = xml_find_child_element(chan, cfgdoc, "Size");
        int chan_size = atoi(xml_element_get_text(size));

        lsi_ctl->m_chan[i * 2].m_from = lsi_addr_1;
        lsi_ctl->m_chan[i * 2].m_to = lsi_addr_2;
        lsi_ctl->m_chan[i * 2].m_size = ROUNDUP_POWOF_2(chan_size);

        lsi_ctl->m_chan[i * 2 + 1].m_from = lsi_addr_2;
        lsi_ctl->m_chan[i * 2 + 1].m_to = lsi_addr_1;
        lsi_ctl->m_chan[i * 2 + 1].m_size = ROUNDUP_POWOF_2(chan_size);

        lsi_ctl->m_head.m_size += (ROUNDUP_POWOF_2(chan_size) + sizeof(LsiChanHead)) * 2;
        i ++;
    }

    return lsi_ctl;
}

int lsi_ctl_init(LsiCtl* lsi_ctl)
{
    if (!lsi_ctl || lsi_ctl->m_head.m_size <= 0)
    {
        return -1;
    }

    int sum, shmid, last_chan_size, page_size, i;
    char* pool, *chan;

    // create share memory
    page_size = getpagesize();
    sum = MemAlign(lsi_ctl->m_head.m_size, page_size);
    shmid = shmget(lsi_ctl->m_head.m_shm_key, sum, 0666 | IPC_CREAT | IPC_EXCL);
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
    pool = (char*)(shmat(shmid, 0, 0));
    memset(pool, 0, sum);
    memcpy(pool, &lsi_ctl->m_head, sizeof(lsi_ctl->m_head));

    chan = pool + sizeof(lsi_ctl->m_head);
    last_chan_size = 0;
    for (i = 0; i < lsi_ctl->m_head.m_chan_count; i++)
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

    int shmid, last_chan_size, i;
    char* pool, *chan;
    LsiHead* load_head;
    LsiChanHead* chan_head;

    // get share memory
    shmid = shmget(lsi_ctl->m_head.m_shm_key, 0, 0666);
    if (shmid < 0)
    {
        printf("LSI [%d] get fail\n", lsi_ctl->m_head.m_shm_key);
        return -1;
    }

    // check data
    pool = (char*)(shmat(shmid, 0, 0));
    load_head = (LsiHead*)pool;
    if (memcmp(load_head, &lsi_ctl->m_head, sizeof(LsiHead)))
    {
        printf("LSI [%d] head fail\n", lsi_ctl->m_head.m_shm_key);
        return -1;
    }

    chan = pool + sizeof(*load_head);
    last_chan_size = 0;
    for (i = 0; i < lsi_ctl->m_head.m_chan_count; i++)
    {
        chan += last_chan_size;
        chan_head = (LsiChanHead*)(chan);
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
    for (i = 0; i < lsi_ctl->m_head.m_chan_count; i++)
    {
        chan += last_chan_size;
        chan_head = (LsiChanHead*)(chan);
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
    free(lsi_ctl);
    lsi_ctl = NULL;
    return 0;
}

