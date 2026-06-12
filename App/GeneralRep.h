#ifndef GENERALREP_H
#define GENERALREP_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "Checkpoint.h"
#include "Hash.h"
#include "ProposalCommon.h"
#include "ProposalFast.h"
#include "Justification.h"

class GeneralRep
{
private:
	Hash lockHash;			  // Hash of the last locked block
	View lockView;			  // View of [lockHash]
	Hash prepareHash;		  // Hash of the last prepared block
	View prepareView;		  // View of [prepareHash]
	View view;				  // Current view
	Phase phase;			  // Current phase
	Path path;				  // Current path
	ReplicaID replicaId;	  // Unique identifier
	Key privateKey;			  // Private key
	Checkpoint checkpoint;	  // Checkpoint
	Quorum generalQuorumSize; // General quorum size
	Quorum trustedQuorumSize; // Trusted quorum size

	void incrementCommon();
	void incrementFast();
	Sign signText(std::string text);
	Justification updateRoundDataCommon(Hash hash1, Hash hash2, View view);
	Justification updateRoundDataFast(Hash hash1, Hash hash2, View view);
	bool verifySigns(Signs signs, ReplicaID replicaId, Nodes nodes, std::string text);

public:
	GeneralRep();
	GeneralRep(ReplicaID replicaId, Key privateKey, Quorum generalQuorumSize, Quorum trustedQuorumSize);

	bool verifyJustification(Nodes nodes, Justification justification);
	bool verifyProposalCommon(Nodes nodes, ProposalCommon proposal, Signs signs);
	bool verifyProposalFast(Nodes nodes, ProposalFast proposal, Signs signs);

	void common2fast();
	void fast2common();
	void initializeCheckpoint(Hash proposeHash, View proposeView);
	void updateCheckpoint(Hash verifyHash, View verifyView, Validations validations, Signs signs);
	Validation checkBlock(Nodes nodes, Justification justification, bool isFail);

	// Common ResiBFT
	Justification initializeMsgNewviewCommon();
	Justification respondProposalCommon(Nodes nodes, Hash proposeHash, Justification justification_MsgNewviewCommon);
	Signs initializeMsgLdrprepareCommon(ProposalCommon proposal_MsgLdrprepareCommon);
	Justification saveMsgPrepareCommon(Nodes nodes, Justification justification_MsgPrepareCommon);
	Justification lockMsgPrecommitCommon(Nodes nodes, Justification justification_MsgPrecommitCommon);

	// Fast ResiBFT
	Justification initializeMsgNewviewFast();

	// Fast to common ResiBFT
	Justification respondProposalFast2Common(Nodes nodes, Hash proposeHash, Justification justification_MsgNewviewFast);
};

#endif
