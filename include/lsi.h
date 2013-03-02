#ifndef LSI_H_
#define LSI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/types.h>

typedef uint32_t lsi_ip_t;
typedef int32_t lsi_id_t;

#define MAX_LSI_CHAN_COUNT  100
#define INVALID_LSI_IP 0

struct LSI;

// error code
enum LSI_ErrCode
{
    LSI_Succ = 0,
    LSI_NoChannel = -100,
    LSI_ChannFull,
    LSI_ChannEmpty,
    LSI_BufferNotEnough,
    LSI_Fail,
};

struct LSI* lsi_create(lsi_id_t lsi_id, lsi_ip_t addr, int lsi_version);

int lsi_destroy(struct LSI* lsi);

// return 0: send success
//  LSI_NoChannel: no send channel find
//  LSI_ChannFull: send channel is full fail
//  LSI_Fail: other fail
int lsi_send(struct LSI* lsi, lsi_ip_t to, const char* send_buf, size_t buf_len);

// return > 0: recv bytes
// return LSI_NoChannel: no send channel find
// return LSI_ChannEmpty: receive channel is empty, no data
// @buf_len: input & output.
int lsi_recv(struct LSI* lsi, lsi_ip_t from, char* recv_buf, size_t* buf_len);

const char* lsi_addr_ntoa(lsi_ip_t lsi_addr);

lsi_ip_t lsi_addr_aton(const char* lsi_addr_str);

#ifdef __cplusplus
}
#endif

#endif // LSI_H_

