#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/shm.h>
#include "lsi.h"
#include "lsi_tool.h"

int lsi_chan_recv_hash(const void* data)
{
    LsiChanHead* chan = (LsiChanHead*)data;
    return chan->m_from;
}

int lsi_chan_recv_cmp(const void* data1, const void* data2)
{
    LsiChanHead* chan1 = (LsiChanHead*)data1;
    LsiChanHead* chan2 = (LsiChanHead*)data2;
    return chan1->m_from - chan2->m_from;
}

int lsi_chan_send_hash(const void* data)
{
    LsiChanHead* chan = (LsiChanHead*)data;
    return chan->m_to;
}

int lsi_chan_send_cmp(const void* data1, const void* data2)
{
    LsiChanHead* chan1 = (LsiChanHead*)data1;
    LsiChanHead* chan2 = (LsiChanHead*)data2;
    return chan1->m_to - chan2->m_to;
}

// ensure enough to write
void lsi_chan_send(LsiChanHead* chan, const char* buffer, int buf_len)
{
    int size = chan->m_size;
	int start_idx = chan->start_idx;
	
	// real end index should be recorded end_idx - 1
	int end_idx = (chan->end_idx - 1 + size) % size;
    char* channel = (char*)chan + sizeof(LsiChanHead);

    // 1. buffer tail is not enough to write, append to buffer head
    if (start_idx <= end_idx && size - end_idx < buf_len)
    {
        int sub_len_tail = size - end_idx;
        int sub_len_head = buf_len - sub_len_tail;
        memcpy(channel + end_idx, buffer, sub_len_tail);
        memcpy(channel, buffer + sub_len_tail, sub_len_head);
        chan->end_idx = (sub_len_head + 1) % size;
    }
	
    // 2. only write new buffer in the tail of queue
    else
    {
        memcpy(channel + end_idx, buffer, buf_len);
        chan->end_idx += buf_len;
		chan->end_idx %= size;
    }

    // statics data
    chan->write_bytes += buf_len;
}

// ensure enough to read
void lsi_chan_recv(LsiChanHead* chan, char* buffer, int buf_len)
{
    int start_idx = chan->start_idx;
    int tail_len = chan->m_size - start_idx;
    char* channel = (char*)chan + sizeof(LsiChanHead);

    if (buf_len < tail_len)
    {
        memcpy(buffer, channel + start_idx, buf_len);
        chan->start_idx += buf_len;
    }
    else
    {
        memcpy(buffer, channel + start_idx, tail_len);
        memcpy(buffer + tail_len, channel, buf_len - tail_len);
        chan->start_idx = buf_len - tail_len;
    }

    // statics data
    chan->read_bytes += buf_len;
}

// ensure enough to read
void lsi_chan_peek(LsiChanHead* chan, char* buffer, int buf_len)
{
    int start_idx = chan->start_idx;
    int tail_len = chan->m_size - start_idx;
    char* channel = (char*)chan + sizeof(LsiChanHead);

    if (buf_len < tail_len)
    {
        memcpy(buffer, channel + start_idx, buf_len);
    }
    else
    {
        memcpy(buffer, channel + start_idx, tail_len);
        memcpy(buffer + tail_len, channel, buf_len - tail_len);
    }
}

// we set default:
//  end == start + 1
// if channel full, then start == end
int lsi_chan_remain_send_bytes(LsiChanHead* chan)
{
    int start_idx = chan->start_idx;
    int end_idx = chan->end_idx;
    return (start_idx + chan->m_size - end_idx) % chan->m_size;
}

int lsi_chan_remain_recv_bytes(LsiChanHead* chan)
{
    return chan->m_size - 1 - lsi_chan_remain_send_bytes(chan);
}

const char* lsi_addr_ntoa(lsi_ip_t lsi_addr)
{
    static struct in_addr addr;
    memcpy(&addr, &lsi_addr, sizeof(lsi_addr));
    return inet_ntoa(addr);
}

lsi_ip_t lsi_addr_aton(const char* lsi_addr_str)
{
    if (lsi_addr_str)
    {
        return  inet_addr(lsi_addr_str);
    }
    return INVALID_LSI_IP;
}

LSI* lsi_create(lsi_id_t lsi_id, lsi_ip_t addr, int lsi_version)
{
    int shmid = 0;
    LsiHead* head = NULL;
    char* from = 0;
    int i = 0;
    LsiChanHead* chan_head = NULL;

    LSI* lsi = (LSI*)malloc(sizeof(LSI));
    if (!lsi)
    {
        return NULL;
    }

    memset(lsi, 0, sizeof(lsi));
    lsi->m_addr = addr;
    lsi->m_lsi_id = lsi_id;
    lsi->m_version = lsi_version;

    // recv & send channel hash table
    lsi->m_recv_chan = hash_create(lsi_chan_recv_hash, lsi_chan_recv_cmp, MAX_LSI_CHAN_COUNT * 3);
    if (!lsi->m_recv_chan)
    {
        goto CreateFail;
    }
    lsi->m_send_chan = hash_create(lsi_chan_send_hash, lsi_chan_send_cmp, MAX_LSI_CHAN_COUNT * 3);
    if (!lsi->m_send_chan)
    {
        goto CreateFail;
    }

    // get share momory
    shmid = shmget(lsi_id, 0, 0666);
    if (shmid < 0)
    {
        goto CreateFail;
    }
    lsi->m_shm = (char*)shmat(shmid, 0, 0);
    if (!lsi->m_shm)
    {
        goto CreateFail;
    }

    // check shm head
    head = (LsiHead*)(lsi->m_shm);
    if (head->m_version != lsi->m_version)
    {
        printf("shm version=%d out-of-date\n", head->m_version);
        goto CreateFail;
    }

    // loop each channel
    from = (char*)head + sizeof(LsiHead);
    for (i = 0; i < head->m_chan_count; i++)
    {
        chan_head = (LsiChanHead*)(from);
        if (chan_head->m_from == lsi->m_addr)
        {
            printf("get send channel: [%s]->", lsi_addr_ntoa(chan_head->m_from));
	   printf("[%s]\n", lsi_addr_ntoa(chan_head->m_to));
            hash_insert(lsi->m_send_chan, chan_head);
        }
        if (chan_head->m_to == lsi->m_addr)
        {
            printf("get recv channel: [%s]->", lsi_addr_ntoa(chan_head->m_from));
	   printf("[%s]\n", lsi_addr_ntoa(chan_head->m_to));
            hash_insert(lsi->m_recv_chan, chan_head);
        }
        from = (char*)chan_head + sizeof(LsiChanHead) + chan_head->m_size;
    }

    return lsi;

CreateFail:
    if (lsi->m_recv_chan)
    {
        hash_destroy(lsi->m_recv_chan);
    }
    if (lsi->m_send_chan)
    {
        hash_destroy(lsi->m_send_chan);
    }
    free(lsi);
    lsi = NULL;

    return NULL;
}

int lsi_destroy(LSI* lsi)
{
    if (!lsi)
    {
        return LSI_Fail;
    }

    if (lsi->m_recv_chan)
    {
        hash_destroy(lsi->m_recv_chan);
    }
    if (lsi->m_send_chan)
    {
        hash_destroy(lsi->m_send_chan);
    }
    free(lsi);
    lsi = NULL;
    return LSI_Succ;
}

// return 0: send success
//  LSI_NoChannel: no send channel find
//  LSI_ChannFull: send channel is full fail
//  LSI_Fail: other fail
int lsi_send(LSI* lsi, lsi_ip_t to, const char* send_buf, int buf_len)
{
    if (!lsi || !lsi->m_send_chan || buf_len <= 0)
    {
        return LSI_Fail;
    }

    LsiChanHead chan;
    chan.m_to = to;
    LsiChanHead* dest = (LsiChanHead*)hash_find(lsi->m_send_chan, &chan);
    if (!dest)
    {
        return LSI_NoChannel;
    }

    int remains = lsi_chan_remain_send_bytes(dest);
    if (remains < (int)(buf_len + sizeof(int)))
    {
        return LSI_ChannFull;
    }

    lsi_chan_send(dest, (char*)&buf_len, sizeof(buf_len));
    lsi_chan_send(dest, send_buf, buf_len);
    return LSI_Succ;
}

// return > 0: recv bytes
// return LSI_NoChannel: no send channel find
// return LSI_ChannEmpty: receive channel is empty, no data
// @buf_len: input & output.
int lsi_recv(LSI* lsi, lsi_ip_t from, char* recv_buf, int* buf_len)
{
    if (!lsi || !lsi->m_recv_chan || buf_len <= 0 || !buf_len)
    {
        return LSI_Fail;
    }

    LsiChanHead chan;
    chan.m_from = from;
    LsiChanHead* dest = (LsiChanHead*)hash_find(lsi->m_recv_chan, &chan);
    if (!dest)
    {
        return LSI_NoChannel;
    }

    int recv_len = lsi_chan_remain_recv_bytes(dest);
    if (recv_len < (int)sizeof(int))
    {
        return LSI_ChannEmpty;
    }

    // read a length, then check data in queue ready for read
    int length = 0;
    lsi_chan_peek(dest, (char*)&length, sizeof(length));

    // data length error
    if (length < 0 || length > dest->m_size)
    {
        return LSI_Fail;
    }

    if (length > *buf_len)
    {
        return LSI_BufferNotEnough;
    }

    // not complete data
    if (recv_len < (int)(length + sizeof(int)))
    {
        return LSI_ChannEmpty;
    }

    // recv data
    lsi_chan_recv(dest, (char*)&length, sizeof(length));
    *buf_len = length;
    lsi_chan_recv(dest, recv_buf, length);

    return LSI_Succ;
}




