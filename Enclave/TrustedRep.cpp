#include <set>
#include "EnclaveBasic.h"

Hash_t prepareHash_t = initiateHash_t(); // Hash of the last prepared block
View prepareView_t = 0;					 // View of [prepareHash_t]
View view_t = 0;						 // Current view
Phase phase_t = PHASE_NEWVIEW_COMMON;	 // Current phase
Path path_t = COMMON_PATH;				 // Current path
Checkpoint_t checkpoint_t;				 // Checkpoint

void incrementCommon_t()
{
	if (phase_t == PHASE_NEWVIEW_COMMON)
	{
		phase_t = PHASE_PREPARE_COMMON;
	}
	else if (phase_t == PHASE_PREPARE_COMMON)
	{
		phase_t = PHASE_PRECOMMIT_COMMON;
	}
	else if (phase_t == PHASE_PRECOMMIT_COMMON)
	{
		phase_t = PHASE_COMMIT_COMMON;
	}
	else if (phase_t == PHASE_COMMIT_COMMON)
	{
		phase_t = PHASE_NEWVIEW_COMMON;
		view_t++;
	}
}

void incrementFast_t()
{
	if (phase_t == PHASE_NEWVIEW_FAST)
	{
		phase_t = PHASE_PREPARE_FAST;
	}
	else if (phase_t == PHASE_PREPARE_FAST)
	{
		phase_t = PHASE_PRECOMMIT_FAST;
	}
	else if (phase_t == PHASE_PRECOMMIT_FAST)
	{
		phase_t = PHASE_NEWVIEW_FAST;
		view_t++;
	}
}

Justification_t updateRoundDataCommon_t(Hash_t hash1, Hash_t hash2, View view)
{
	RoundData_t roundData_t;
	roundData_t.proposeHash = hash1;
	roundData_t.proposeView = view_t;
	roundData_t.justifyHash = hash2;
	roundData_t.justifyView = view;
	roundData_t.phase = phase_t;
	Sign_t sign_t = signData_t(roundData2string_t(roundData_t));
	Signs_t signs_t;
	signs_t.size = 1;
	signs_t.signs[0] = sign_t;
	Justification_t justification_t;
	justification_t.set = true;
	justification_t.roundData = roundData_t;
	justification_t.signs = signs_t;
	incrementCommon_t();
	return justification_t;
}

Justification_t updateRoundDataFast_t(Hash_t hash1, Hash_t hash2, View view)
{
	RoundData_t roundData_t;
	roundData_t.proposeHash = hash1;
	roundData_t.proposeView = view_t;
	roundData_t.justifyHash = hash2;
	roundData_t.justifyView = view;
	roundData_t.phase = phase_t;
	Sign_t sign_t = signData_t(roundData2string_t(roundData_t));
	Signs_t signs_t;
	signs_t.size = 1;
	signs_t.signs[0] = sign_t;
	Justification_t justification_t;
	justification_t.set = true;
	justification_t.roundData = roundData_t;
	justification_t.signs = signs_t;
	incrementFast_t();
	return justification_t;
}

sgx_status_t TEE_verifyJustification(Justification_t *justification_t, bool *b)
{
	sgx_status_t status_t = SGX_SUCCESS;

	*b = verifyJustification_t(justification_t);

	return status_t;
}

sgx_status_t TEE_verifyProposalCommon(ProposalCommon_t *proposalCommon_t, Signs_t *signs_t, bool *b)
{
	sgx_status_t status_t = SGX_SUCCESS;

	*b = verifyProposalCommon_t(proposalCommon_t, signs_t);

	return status_t;
}

sgx_status_t TEE_verifyProposalFast(ProposalFast_t *proposalFast_t, Signs_t *signs_t, bool *b)
{
	sgx_status_t status_t = SGX_SUCCESS;

	*b = verifyProposalFast_t(proposalFast_t, signs_t);

	return status_t;
}

sgx_status_t TEE_common2fast()
{
	sgx_status_t status_t = SGX_SUCCESS;

	phase_t = PHASE_NEWVIEW_FAST;

	return status_t;
}

sgx_status_t TEE_fast2common()
{
	sgx_status_t status_t = SGX_SUCCESS;

	phase_t = PHASE_PREPARE_COMMON;

	return status_t;
}

sgx_status_t TEE_initializeCheckpoint(Hash_t *proposeHash_t, View *proposeView)
{
	sgx_status_t status_t = SGX_SUCCESS;

	checkpoint_t.verifyHash = *proposeHash_t;
	checkpoint_t.verifyView = *proposeView;

	return status_t;
}

sgx_status_t TEE_updateCheckpoint(Hash_t *verifyHash_t, View *verifyView, Validations_t *validations_t, Signs_t *signs_t)
{
	sgx_status_t status_t = SGX_SUCCESS;

	checkpoint_t.verifyHash = *verifyHash_t;
	checkpoint_t.verifyView = *verifyView;
	checkpoint_t.validations = *validations_t;
	checkpoint_t.signs = *signs_t;

	return status_t;
}

sgx_status_t TEE_checkBlock(Justification_t *justification_t, bool *isFail, Validation_t *validation_t)
{
	sgx_status_t status_t = SGX_SUCCESS;

	RoundData_t roundData_t = justification_t->roundData;
	View proposeView = roundData_t.proposeView;
	if (verifyJustification_t(justification_t) && proposeView >= checkpoint_t.verifyView && !isFail)
	{
		validation_t->set = true;
		validation_t->verifier = true;
	}
	else
	{
		validation_t->set = true;
		validation_t->verifier = false;
	}

	return status_t;
}

// Common ResiBFT
sgx_status_t TEE_initializeMsgNewviewCommon(Justification_t *justification_MsgNewviewCommon_t)
{
	sgx_status_t status_t = SGX_SUCCESS;

	Hash_t hash_t = initiateDummyHash_t();
	*justification_MsgNewviewCommon_t = updateRoundDataCommon_t(hash_t, prepareHash_t, prepareView_t);

	return status_t;
}

sgx_status_t TEE_respondProposalCommon(Hash_t *proposeHash_t, Justification_t *justification_MsgNewviewCommon_t, Justification_t *justification_MsgPrepareCommon_t)
{
	sgx_status_t status_t = SGX_SUCCESS;

	RoundData_t roundData_MsgNewviewCommon_t = justification_MsgNewviewCommon_t->roundData;
	View proposeView_MsgNewviewCommon = roundData_MsgNewviewCommon_t.proposeView;
	Hash_t justifyHash_MsgNewviewCommon_t = roundData_MsgNewviewCommon_t.justifyHash;
	View justifyView_MsgNewviewCommon_t = roundData_MsgNewviewCommon_t.justifyView;
	Phase phase_MsgNewviewCommon_t = roundData_MsgNewviewCommon_t.phase;
	if (verifyJustification_t(justification_MsgNewviewCommon_t) && view_t == proposeView_MsgNewviewCommon && phase_MsgNewviewCommon_t == PHASE_NEWVIEW_COMMON)
	{
		*justification_MsgPrepareCommon_t = updateRoundDataCommon_t(*proposeHash_t, justifyHash_MsgNewviewCommon_t, justifyView_MsgNewviewCommon_t);
	}
	else
	{
		justification_MsgPrepareCommon_t->set = false;
		if (DEBUG_TEE)
		{
			TEE_Print((printReplicaId_t() + " fail to respond proposal in common path").c_str());
		}
	}

	return status_t;
}

sgx_status_t TEE_initializeMsgLdrprepareCommon(ProposalCommon_t *proposalCommon_MsgLdrprepareCommon_t, Signs_t *signs_MsgLdrprepareCommon_t)
{
	sgx_status_t status_t = SGX_SUCCESS;

	Sign_t sign_MsgLdrprepareFast_t = signData_t(proposalCommon2string_t(*proposalCommon_MsgLdrprepareCommon_t));
	signs_MsgLdrprepareCommon_t->size = 1;
	signs_MsgLdrprepareCommon_t->signs[0] = sign_MsgLdrprepareFast_t;

	return status_t;
}

sgx_status_t TEE_saveMsgPrepareCommon(Justification_t *justification_MsgPrepareCommon_t, Justification_t *justification_MsgPrecommitCommon_t)
{
	sgx_status_t status_t = SGX_SUCCESS;

	RoundData_t roundData_MsgPrepareCommon_t = justification_MsgPrepareCommon_t->roundData;
	Hash_t proposeHash_MsgPrepareCommon_t = roundData_MsgPrepareCommon_t.proposeHash;
	View proposeView_MsgPrepareCommon_t = roundData_MsgPrepareCommon_t.proposeView;
	Phase phase_MsgPrepareCommon_t = roundData_MsgPrepareCommon_t.phase;
	if (verifyJustification_t(justification_MsgPrepareCommon_t) && justification_MsgPrepareCommon_t->signs.size == getGeneralQuorumSize_t() && view_t == proposeView_MsgPrepareCommon_t && phase_MsgPrepareCommon_t == PHASE_PREPARE_COMMON)
	{
		prepareHash_t = proposeHash_MsgPrepareCommon_t;
		prepareView_t = proposeView_MsgPrepareCommon_t;
		*justification_MsgPrecommitCommon_t = updateRoundDataCommon_t(proposeHash_MsgPrepareCommon_t, initiateHash_t(), 0);
	}
	else
	{
		justification_MsgPrecommitCommon_t->set = false;
		if (DEBUG_TEE)
		{
			TEE_Print((printReplicaId_t() + " fail to save in MsgPrepare in common path").c_str());
		}
	}

	return status_t;
}

sgx_status_t TEE_lockMsgPrecommitCommon(Justification_t *justification_MsgPrecommitCommon_t, Justification_t *justification_MsgCommitCommon_t)
{
	sgx_status_t status_t = SGX_SUCCESS;

	RoundData_t roundData_MsgPrecommitCommon_t = justification_MsgPrecommitCommon_t->roundData;
	Hash_t proposeHash_MsgPrecommitCommon_t = roundData_MsgPrecommitCommon_t.proposeHash;
	View proposeView_MsgPrecommitCommon_t = roundData_MsgPrecommitCommon_t.proposeView;
	Phase phase_MsgPrecommitCommon_t = roundData_MsgPrecommitCommon_t.phase;
	if (verifyJustification_t(justification_MsgPrecommitCommon_t) && justification_MsgPrecommitCommon_t->signs.size == getGeneralQuorumSize_t() && view_t == proposeView_MsgPrecommitCommon_t && phase_MsgPrecommitCommon_t == PHASE_PRECOMMIT_COMMON)
	{
		prepareHash_t = proposeHash_MsgPrecommitCommon_t;
		prepareView_t = proposeView_MsgPrecommitCommon_t;
		*justification_MsgCommitCommon_t = updateRoundDataCommon_t(proposeHash_MsgPrecommitCommon_t, initiateHash_t(), 0);
	}
	else
	{
		justification_MsgCommitCommon_t->set = false;
		if (DEBUG_TEE)
		{
			TEE_Print((printReplicaId_t() + " fail to lock in MsgPrecommit in common path").c_str());
		}
	}

	return status_t;
}

// Fast ResiBFT
sgx_status_t TEE_initializeMsgNewviewFast(Justification_t *justification_MsgNewviewFast_t)
{
	sgx_status_t status_t = SGX_SUCCESS;

	Hash_t hash_t = initiateDummyHash_t();
	*justification_MsgNewviewFast_t = updateRoundDataFast_t(hash_t, prepareHash_t, prepareView_t);

	return status_t;
}

sgx_status_t TEE_initializeAccumulatorFast(Justifications_t *justifications_MsgNewviewFast_t, Accumulator_t *accumulator_MsgLdrprepareFast_t)
{
	sgx_status_t status_t = SGX_SUCCESS;

	View proposeView_MsgNewviewFast = justifications_MsgNewviewFast_t->justifications[0].roundData.proposeView;
	View highView = 0;
	Hash_t highHash_t = initiateHash_t();
	std::set<ReplicaID> signers;

	for (int i = 0; i < NUM_ACTIVE_REPLICAS; i++)
	{
		Justification_t justification_MsgNewviewFast_t = justifications_MsgNewviewFast_t->justifications[i];
		RoundData_t roundData_MsgNewviewFast_t = justification_MsgNewviewFast_t.roundData;
		View justifyView_MsgNewviewFast = roundData_MsgNewviewFast_t.justifyView;
		Hash_t justifyHash_MsgNewviewFast_t = roundData_MsgNewviewFast_t.justifyHash;
		Signs_t signs_MsgNewviewFast_t = justification_MsgNewviewFast_t.signs;
		ReplicaID signer = signs_MsgNewviewFast_t.signs[0].signer;
		if (verifyJustification_t(&justification_MsgNewviewFast_t) && roundData_MsgNewviewFast_t.proposeView == proposeView_MsgNewviewFast && roundData_MsgNewviewFast_t.phase == PHASE_NEWVIEW_FAST)
		{
			if (signers.find(signer) == signers.end())
			{
				signers.insert(signer);
				if (justifyView_MsgNewviewFast >= highView)
				{
					highView = justifyView_MsgNewviewFast;
					highHash_t = justifyHash_MsgNewviewFast_t;
				}
			}
		}
	}

	accumulator_MsgLdrprepareFast_t->set = true;
	accumulator_MsgLdrprepareFast_t->proposeView = proposeView_MsgNewviewFast;
	accumulator_MsgLdrprepareFast_t->prepareHash = highHash_t;
	accumulator_MsgLdrprepareFast_t->prepareView = highView;
	accumulator_MsgLdrprepareFast_t->size = signers.size();

	return status_t;
}

sgx_status_t TEE_respondProposalFast(Hash_t *proposeHash_t, Accumulator_t *accumulator_MsgLdrprepareFast_t, Justification_t *justification_MsgPrepareFast_t)
{
	sgx_status_t status_t = SGX_SUCCESS;

	View proposeView_MsgLdrprepareFast = accumulator_MsgLdrprepareFast_t->proposeView;
	Hash_t prepareHash_MsgLdrprepareFast_t = accumulator_MsgLdrprepareFast_t->prepareHash;
	View prepareView_MsgLdrprepareFast = accumulator_MsgLdrprepareFast_t->prepareView;
	unsigned int size_MsgLdrprepareFast = accumulator_MsgLdrprepareFast_t->size;
	if (view_t == proposeView_MsgLdrprepareFast && size_MsgLdrprepareFast == getGeneralQuorumSize_t())
	{
		*justification_MsgPrepareFast_t = updateRoundDataFast_t(*proposeHash_t, prepareHash_MsgLdrprepareFast_t, prepareView_MsgLdrprepareFast);
	}
	else
	{
		justification_MsgPrepareFast_t->set = false;
		if (DEBUG_TEE)
		{
			TEE_Print((printReplicaId_t() + " fail to respond accumulator in fast path").c_str());
		}
	}

	return status_t;
}

sgx_status_t TEE_initializeMsgLdrprepareFast(ProposalFast_t *proposalFast_MsgLdrprepareFast_t, Signs_t *signs_MsgLdrprepareFast_t)
{
	sgx_status_t status_t = SGX_SUCCESS;

	Sign_t sign_MsgLdrprepareFast_t = signData_t(proposalFast2string_t(*proposalFast_MsgLdrprepareFast_t));
	signs_MsgLdrprepareFast_t->size = 1;
	signs_MsgLdrprepareFast_t->signs[0] = sign_MsgLdrprepareFast_t;

	return status_t;
}

sgx_status_t TEE_saveMsgPrepareFast(Justification_t *justification_MsgPrepareFast_t, bool *isFail_MsgPrepareFast, Justification_t *justification_MsgPrecommitFast_t)
{
	sgx_status_t status_t = SGX_SUCCESS;

	RoundData_t roundData_MsgPrepareFast_t = justification_MsgPrepareFast_t->roundData;
	Hash_t proposeHash_MsgPrepareFast_t = roundData_MsgPrepareFast_t.proposeHash;
	View proposeView_MsgPrepareFast_t = roundData_MsgPrepareFast_t.proposeView;
	Phase phase_MsgPrepareFast_t = roundData_MsgPrepareFast_t.phase;
	if (verifyJustification_t(justification_MsgPrepareFast_t) && justification_MsgPrepareFast_t->signs.size == getTrustedQuorumSize_t() && view_t == proposeView_MsgPrepareFast_t && phase_MsgPrepareFast_t == PHASE_PREPARE_FAST)
	{
		if (!isFail_MsgPrepareFast)
		{
			prepareHash_t = proposeHash_MsgPrepareFast_t;
			prepareView_t = proposeView_MsgPrepareFast_t;
			*justification_MsgPrecommitFast_t = updateRoundDataFast_t(proposeHash_MsgPrepareFast_t, initiateHash_t(), 0);
		}
		else
		{
			*justification_MsgPrecommitFast_t = updateRoundDataFast_t(proposeHash_MsgPrepareFast_t, initiateHash_t(), 0);
		}
	}
	else
	{
		justification_MsgPrecommitFast_t->set = false;
		if (DEBUG_TEE)
		{
			TEE_Print((printReplicaId_t() + " fail to save in MsgPrepare in fast path").c_str());
		}
	}

	return status_t;
}

// Fast to common ResiBFT
sgx_status_t TEE_respondProposalFast2Common(Hash_t *proposeHash_t, Justification_t *justification_MsgNewviewFast_t, Justification_t *justification_MsgPrepareCommon_t)
{
	sgx_status_t status_t = SGX_SUCCESS;

	RoundData_t roundData_MsgNewviewFast_t = justification_MsgNewviewFast_t->roundData;
	View proposeView_MsgNewviewFast = roundData_MsgNewviewFast_t.proposeView;
	Hash_t justifyHash_MsgNewviewFast_t = roundData_MsgNewviewFast_t.justifyHash;
	View justifyView_MsgNewviewFast_t = roundData_MsgNewviewFast_t.justifyView;
	Phase phase_MsgNewviewFast_t = roundData_MsgNewviewFast_t.phase;
	if (verifyJustification_t(justification_MsgNewviewFast_t) && view_t == proposeView_MsgNewviewFast && phase_MsgNewviewFast_t == PHASE_NEWVIEW_FAST)
	{
		*justification_MsgPrepareCommon_t = updateRoundDataCommon_t(*proposeHash_t, justifyHash_MsgNewviewFast_t, justifyView_MsgNewviewFast_t);
	}
	else
	{
		justification_MsgPrepareCommon_t->set = false;
		if (DEBUG_TEE)
		{
			TEE_Print((printReplicaId_t() + " fail to respond proposal in common path").c_str());
		}
	}

	return status_t;
}