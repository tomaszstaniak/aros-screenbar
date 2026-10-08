#ifndef SCREENBAR_PROTOCOL_H
#define SCREENBAR_PROTOCOL_H
#include <stdint.h>
#define SB_PROTOCOL_VERSION 1
#define SB_MAX_PROVIDERS 8
#define SB_LABEL_SIZE 32
#define SB_TEXT_SIZE 64
#define SB_LEASE_SECONDS 5
#define SB_ACTION_PRIMARY 1
enum SbOperation { SB_REGISTER=1, SB_UPDATE, SB_UNREGISTER };
enum SbStatus { SB_OK, SB_ABSENT, SB_STALE, SB_REVISION, SB_FULL, SB_INVALID, SB_VERSION, SB_STOPPED };
struct SbRequest {
 uint32_t version,op;
 uint64_t session;
 uint32_t token,revision;
 char label[SB_LABEL_SIZE],text[SB_TEXT_SIZE],action_label[SB_LABEL_SIZE];
};
struct SbResponse {uint32_t status,action;uint64_t session;uint32_t token;};
#endif
