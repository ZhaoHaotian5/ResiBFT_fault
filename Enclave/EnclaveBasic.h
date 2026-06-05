#ifndef ENCLAVEBASIC_H
#define ENCLAVEBASIC_H

#include <map>
#include <string>
#include <openssl/err.h>
#include <openssl/pem.h>
#include "Enclave_t.h"
#include "EnclaveUsertypes.h"
#include "../App/config.h"
#include "../App/key.h"
#include "../App/types.h"

Quorum getGeneralQuorumSize_t();
Quorum getTrustedQuorumSize_t();
std::string printReplicaId_t();
sgx_status_t TEE_initializeVariables(ReplicaID *me, Pids_t *others, unsigned int *GeneralQuorumSize, unsigned int *TrustedQuorumSize);

std::string transaction2string_t(Transaction_t transaction_t);
std::string hash2string_t(Hash_t hash_t);
std::string block2string_t(Block_t block_t);
std::string roundData2string_t(RoundData_t roundData_t);
std::string sign2string_t(Sign_t sign_t);
std::string signs2string_t(Signs_t signs_t);
std::string justification2string_t(Justification_t justification_t);
std::string justifications2string_t(Justifications_t justifications_t);
std::string accumulator2string_t(Accumulator_t accumulator_t);
std::string proposalCommon2string_t(ProposalCommon_t proposalCommon_t);
std::string proposalFast2string_t(ProposalFast_t proposalFast_t);
std::string validation2string_t(Validation_t validation_t);
std::string validations2string_t(Validations_t validations_t);
std::string certificate2string_t(Certificate_t certificate_t);
std::string checkpoint2string_t(Checkpoint_t checkpoint_t);

bool equalHashes_t(Hash_t hash1, Hash_t hash2);
Hash_t initiateHash_t();
Hash_t initiateDummyHash_t();

Sign_t signData_t(std::string text);
bool verify_t(Signs_t signs_t, std::string text);
bool verifyJustification_t(Justification_t *justification_t);
bool verifyProposalCommon_t(ProposalCommon_t *proposalCommon_t, Signs_t *signs_t);
bool verifyProposalFast_t(ProposalFast_t *proposalFast_t, Signs_t *signs_t);

#endif
