#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef uint8_t HEADER;

#define HEADER_TRANSACTION 0x0
#define HEADER_START 0x1
#define HEADER_REPLY 0x2

#define HEADER_NEWVIEW_RESIBFT_COMMON 0x11
#define HEADER_LDRPREPARE_RESIBFT_COMMON 0x12
#define HEADER_PREPARE_RESIBFT_COMMON 0x13
#define HEADER_PRECOMMIT_RESIBFT_COMMON 0x14
#define HEADER_COMMIT_RESIBFT_COMMON 0x15

#define HEADER_NEWVIEW_RESIBFT_FAST 0x21
#define HEADER_LDRPREPARE_RESIBFT_FAST 0x22
#define HEADER_PREPARE_RESIBFT_FAST 0x23
#define HEADER_PRECOMMIT_RESIBFT_FAST 0x24
#define HEADER_VALIDATION_RESIBFT_FAST 0x25

#define PHASE_NEWVIEW_COMMON 0x31
#define PHASE_PREPARE_COMMON 0x32
#define PHASE_PRECOMMIT_COMMON 0x33
#define PHASE_COMMIT_COMMON 0x34

#define PHASE_NEWVIEW_FAST 0x41
#define PHASE_PREPARE_FAST 0x42
#define PHASE_PRECOMMIT_FAST 0x43
#define PHASE_VALIDATION_FAST 0x42

#define FAST_PATH 0x51
#define COMMON_PATH 0x52

typedef uint8_t Phase;
typedef uint8_t Path;
typedef unsigned int ReplicaID;		// process ids
typedef unsigned int ClientID;		// client ids
typedef unsigned int TransactionID; // transaction ids
typedef unsigned int PortID;
typedef unsigned int View;
typedef unsigned int Quorum;

#endif
