#ifndef USER_TYPES_H
#define USER_TYPES_H

#include <stdbool.h>
#include <openssl/sha.h>
#include "../App/types.h"
#include "../App/config.h"
#include "../App/key.h"

typedef struct _Pids_t
{
	unsigned int num_nodes;
	ReplicaID pids[NUM_REPLICAS];
} Pids_t;

typedef struct _Transaction_t
{
	ClientID clientId;
	TransactionID transactionId;
	unsigned char transactionData[NUM_PAYLOAD_SIZE];
} Transaction_t;

typedef struct _Hash_t
{
	bool set;
	unsigned char hash[SHA256_DIGEST_LENGTH];
} Hash_t;

typedef struct _Block_t
{
	bool set;
	Hash_t previousHash;
	unsigned int size;
	Transaction_t transactions[NUM_TRANSACTIONS];
} Block_t;

typedef struct _RoundData_t
{
	Hash_t proposeHash;
	View proposeView;
	Hash_t justifyHash;
	View justifyView;
	Phase phase;
} RoundData_t;

typedef struct _Sign_t
{
	bool set;
	ReplicaID signer;
	unsigned char signtext[SIGN_LEN];
} Sign_t;

typedef struct _Signs_t
{
	unsigned int size;
	Sign_t signs[NUM_ACTIVE_REPLICAS];
} Signs_t;

typedef struct _Justification_t
{
	bool set;
	RoundData_t roundData;
	Signs_t signs;
} Justification_t;

typedef struct _Justifications_t
{
	Justification_t justifications[NUM_ACTIVE_REPLICAS];
} Justifications_t;

typedef struct _Accumulator_t
{
	bool set;
	View proposeView;
	Hash_t prepareHash;
	View prepareView;
	unsigned int size;
} Accumulator_t;

typedef struct _ProposalCommon_t
{
	Justification_t justification;
	Block_t block;
} ProposalCommon_t;

typedef struct _ProposalFast_t
{
	Accumulator_t accumulator;
	Block_t block;
} ProposalFast_t;

typedef struct _Validation_t
{
	bool set;
	bool verifier;
} Validation_t;

typedef struct _Validations_t
{
	unsigned int size;
	Validation_t validations[NUM_REPLICAS];
} Validations_t;

typedef struct _Committee_t
{
	bool set;
	unsigned int size;
	ReplicaID committee[NUM_COMMITTEE_MEMBERS];
} Committee_t;

typedef struct _Certificate_t
{
	Block_t block;
	Validations_t validations;
	Signs_t signs;
} Certificate_t;

typedef struct _Checkpoint_t
{
	Hash_t verifyHash;
	View verifyView;
	Validations_t validations;
	Signs_t signs;
} Checkpoint_t;

#endif
