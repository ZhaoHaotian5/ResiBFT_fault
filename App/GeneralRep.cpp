#include "GeneralRep.h"

void GeneralRep::incrementCommon()
{
	if (this->phase == PHASE_NEWVIEW_COMMON)
	{
		this->phase = PHASE_PREPARE_COMMON;
	}
	else if (this->phase == PHASE_PREPARE_COMMON)
	{
		this->phase = PHASE_PRECOMMIT_COMMON;
	}
	else if (this->phase == PHASE_PRECOMMIT_COMMON)
	{
		this->phase = PHASE_COMMIT_COMMON;
	}
	else if (this->phase == PHASE_COMMIT_COMMON)
	{
		this->phase = PHASE_NEWVIEW_COMMON;
		this->view++;
	}
}

void GeneralRep::incrementFast()
{
	if (this->phase == PHASE_NEWVIEW_FAST)
	{
		this->phase = PHASE_VALIDATION_FAST;
	}
	else if (this->phase == PHASE_VALIDATION_FAST)
	{
		this->phase = PHASE_NEWVIEW_FAST;
		this->view++;
	}
}

Sign GeneralRep::signText(std::string text)
{
	Sign sign = Sign(this->privateKey, this->replicaId, text);
	return sign;
}

Justification GeneralRep::updateRoundDataCommon(Hash hash1, Hash hash2, View view)
{
	RoundData roundData = RoundData(hash1, this->view, hash2, view, this->phase);
	Sign sign = this->signText(roundData.toString());
	Justification justification = Justification(roundData, sign);
	this->incrementCommon();
	return justification;
}

Justification GeneralRep::updateRoundDataFast(Hash hash1, Hash hash2, View view)
{
	RoundData roundData = RoundData(hash1, this->view, hash2, view, this->phase);
	Sign sign = this->signText(roundData.toString());
	Justification justification = Justification(roundData, sign);
	this->incrementFast();
	return justification;
}

bool GeneralRep::verifySigns(Signs signs, ReplicaID replicaId, Nodes nodes, std::string text)
{
	bool b = signs.verify(replicaId, nodes, text);
	return b;
}

GeneralRep::GeneralRep()
{
	this->lockHash = Hash(true); // The genesis block
	this->lockView = 0;
	this->prepareHash = Hash(true); // The genesis block
	this->prepareView = 0;
	this->view = 0;
	this->phase = PHASE_NEWVIEW_COMMON;
	this->path = COMMON_PATH;
	this->checkpoint = Checkpoint();
	this->generalQuorumSize = 0;
	this->trustedQuorumSize = 0;
}

GeneralRep::GeneralRep(ReplicaID replicaId, Key privateKey, Quorum generalQuorumSize, Quorum trustedQuorumSize)
{
	this->lockHash = Hash(true); // The genesis block
	this->lockView = 0;
	this->prepareHash = Hash(true); // The genesis block
	this->prepareView = 0;
	this->view = 0;
	this->phase = PHASE_NEWVIEW_COMMON;
	this->path = COMMON_PATH;
	this->checkpoint = Checkpoint();
	this->replicaId = replicaId;
	this->privateKey = privateKey;
	this->generalQuorumSize = generalQuorumSize;
	this->trustedQuorumSize = trustedQuorumSize;
}

bool GeneralRep::verifyJustification(Nodes nodes, Justification justification)
{
	bool b = this->verifySigns(justification.getSigns(), this->replicaId, nodes, justification.getRoundData().toString());
	return b;
}

bool GeneralRep::verifyProposalCommon(Nodes nodes, ProposalCommon proposal, Signs signs)
{
	bool b = this->verifySigns(signs, this->replicaId, nodes, proposal.toString());
	return b;
}

bool GeneralRep::verifyProposalFast(Nodes nodes, ProposalFast proposal, Signs signs)
{
	bool b = this->verifySigns(signs, this->replicaId, nodes, proposal.toString());
	return b;
}

void GeneralRep::common2fast()
{
	this->phase = PHASE_NEWVIEW_FAST;
}

void GeneralRep::fast2common()
{
	this->phase = PHASE_PREPARE_COMMON;
}

void GeneralRep::initializeCheckpoint(Hash proposeHash, View proposeView)
{
	this->checkpoint = Checkpoint(proposeHash, proposeView);
}

void GeneralRep::updateCheckpoint(Hash verifyHash, View verifyView, Validations validations, Signs signs)
{
	this->checkpoint = Checkpoint(verifyHash, verifyView, validations, signs);
}

Validation GeneralRep::checkBlock(Nodes nodes, Justification justification, bool isFail)
{
	RoundData roundData = justification.getRoundData();
	Hash proposeHash = roundData.getProposeHash();
	View proposeView = roundData.getProposeView();
	if (this->verifyJustification(nodes, justification) && proposeView >= this->checkpoint.getVerifyView() && !isFail)
	{
		this->lockHash = proposeHash;
		this->lockView = proposeView;
		bool set_Validation = true;
		bool verifier_Validation = true;
		Validation validation = Validation(set_Validation, verifier_Validation);
		return validation;
	}
	else
	{
		bool set_Validation = true;
		bool verifier_Validation = false;
		Validation validation = Validation(set_Validation, verifier_Validation);
		return validation;
	}
}

// Common ResiBFT
Justification GeneralRep::initializeMsgNewviewCommon()
{
	Justification justification_MsgNewviewCommon = this->updateRoundDataCommon(Hash(false), this->prepareHash, this->prepareView);
	return justification_MsgNewviewCommon;
}

Justification GeneralRep::respondProposalCommon(Nodes nodes, Hash proposeHash, Justification justification_MsgNewviewCommon)
{
	RoundData roundData_MsgNewviewCommon = justification_MsgNewviewCommon.getRoundData();
	View proposeView_MsgNewviewCommon = roundData_MsgNewviewCommon.getProposeView();
	Hash justifyHash_MsgNewviewCommon = roundData_MsgNewviewCommon.getJustifyHash();
	View justifyView_MsgNewviewCommon = roundData_MsgNewviewCommon.getJustifyView();
	Phase phase_MsgNewviewCommon = roundData_MsgNewviewCommon.getPhase();
	if (this->verifyJustification(nodes, justification_MsgNewviewCommon) && this->view == proposeView_MsgNewviewCommon && phase_MsgNewviewCommon == PHASE_NEWVIEW_COMMON && (this->lockHash == justifyHash_MsgNewviewCommon || this->lockView < justifyView_MsgNewviewCommon))
	{
		Justification justification_MsgPrepareCommon = this->updateRoundDataCommon(proposeHash, justifyHash_MsgNewviewCommon, justifyView_MsgNewviewCommon);
		return justification_MsgPrepareCommon;
	}
	else
	{
		if (DEBUG_MODULES)
		{
			std::cout << COLOUR_CYAN << this->replicaId << " fail to respond proposal in common path" << COLOUR_NORMAL << std::endl;
		}
		return Justification();
	}
}

Signs GeneralRep::initializeMsgLdrprepareCommon(ProposalCommon proposal_MsgLdrprepareCommon)
{
	Sign sign_MsgLdrprepareCommon = this->signText(proposal_MsgLdrprepareCommon.toString());
	Signs signs_MsgLdrprepareCommon = Signs(sign_MsgLdrprepareCommon);
	return signs_MsgLdrprepareCommon;
}

Justification GeneralRep::saveMsgPrepareCommon(Nodes nodes, Justification justification_MsgPrepareCommon)
{
	RoundData roundData_MsgPrepareCommon = justification_MsgPrepareCommon.getRoundData();
	Hash proposeHash_MsgPrepareCommon = roundData_MsgPrepareCommon.getProposeHash();
	View proposeView_MsgPrepareCommon = roundData_MsgPrepareCommon.getProposeView();
	Phase phase_MsgPrepareCommon = roundData_MsgPrepareCommon.getPhase();
	if (this->verifyJustification(nodes, justification_MsgPrepareCommon) && justification_MsgPrepareCommon.getSigns().getSize() == this->generalQuorumSize && this->view == proposeView_MsgPrepareCommon && phase_MsgPrepareCommon == PHASE_PREPARE_COMMON)
	{
		this->prepareHash = proposeHash_MsgPrepareCommon;
		this->prepareView = proposeView_MsgPrepareCommon;
		Justification justification_MsgPrecommitCommon = this->updateRoundDataCommon(proposeHash_MsgPrepareCommon, Hash(), View());
		return justification_MsgPrecommitCommon;
	}
	else
	{
		if (DEBUG_MODULES)
		{
			std::cout << COLOUR_CYAN << this->replicaId << " fail to save in MsgPrepare in common path" << COLOUR_NORMAL << std::endl;
		}
		return Justification();
	}
}

Justification GeneralRep::lockMsgPrecommitCommon(Nodes nodes, Justification justification_MsgPrecommitCommon)
{
	RoundData roundData = justification_MsgPrecommitCommon.getRoundData();
	Hash proposeHash_MsgPrecommitCommon = roundData.getProposeHash();
	View proposeView_MsgPrecommitCommon = roundData.getProposeView();
	Phase phase_MsgPrecommitCommon = roundData.getPhase();
	if (this->verifyJustification(nodes, justification_MsgPrecommitCommon) && justification_MsgPrecommitCommon.getSigns().getSize() == this->generalQuorumSize && this->view == proposeView_MsgPrecommitCommon && phase_MsgPrecommitCommon == PHASE_PRECOMMIT_COMMON)
	{
		this->prepareHash = proposeHash_MsgPrecommitCommon;
		this->prepareView = proposeView_MsgPrecommitCommon;
		this->lockHash = proposeHash_MsgPrecommitCommon;
		this->lockView = proposeView_MsgPrecommitCommon;
		if (DEBUG_MODULES)
		{
			std::cout << COLOUR_CYAN << this->replicaId << " locked" << COLOUR_NORMAL << std::endl;
		}
		Justification justification_MsgCommitCommon = this->updateRoundDataCommon(proposeHash_MsgPrecommitCommon, Hash(), View());
		return justification_MsgCommitCommon;
	}
	else
	{
		if (DEBUG_MODULES)
		{
			std::cout << COLOUR_CYAN << this->replicaId << " fail to lock in MsgPrecommit in common path" << COLOUR_NORMAL << std::endl;
		}
		return Justification();
	}
}

// Fast ResiBFT
Justification GeneralRep::initializeMsgNewviewFast()
{
	Justification justification_MsgNewviewFast = this->updateRoundDataFast(Hash(false), this->prepareHash, this->prepareView);
	return justification_MsgNewviewFast;
}