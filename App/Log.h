#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <map>
#include <set>
#include <stdint.h>
#include <stdio.h>
#include "config.h"
#include "message.h"
#include "Justification.h"
#include "ProposalCommon.h"
#include "ProposalFast.h"

class Log
{
private:
	// Common ResiBFT
	std::map<View, std::set<MsgNewviewCommon>> newviewsCommon;
	std::map<View, std::set<MsgLdrprepareCommon>> ldrpreparesCommon;
	std::map<View, std::set<MsgPrepareCommon>> preparesCommon;
	std::map<View, std::set<MsgPrecommitCommon>> precommitsCommon;
	std::map<View, std::set<MsgCommitCommon>> commitsCommon;

	// Fast ResiBFT
	std::map<View, std::set<MsgNewviewFast>> newviewsFast;
	std::map<View, std::set<MsgLdrprepareFast>> ldrpreparesFast;
	std::map<View, std::set<MsgPrepareFast>> preparesFast;
	std::map<View, std::set<MsgPrecommitFast>> precommitsFast;
	std::map<View, std::set<MsgValidationFast>> validationsFast;

	// Fast2common ResiBFT
	std::map<View, std::set<MsgLdrprepareFast2Common>> ldrpreparesFast2Common;
	std::map<View, std::set<MsgPrepareFast2Common>> preparesFast2Common;
	std::map<View, std::set<MsgPrecommitFast2Common>> precommitsFast2Common;
	std::map<View, std::set<MsgCommitFast2Common>> commitsFast2Common;

public:
	Log();

	// Common ResiBFT
	// Return the number of signatures
	unsigned int storeMsgNewviewCommon(MsgNewviewCommon msgNewview);
	unsigned int storeMsgLdrprepareCommon(MsgLdrprepareCommon msgLdrprepare);
	unsigned int storeMsgPrepareCommon(MsgPrepareCommon msgPrepare);
	unsigned int storeMsgPrecommitCommon(MsgPrecommitCommon msgPrecommit);
	unsigned int storeMsgCommitCommon(MsgCommitCommon msgCommit);

	// Collect [n] signatures of the messages
	Signs getMsgNewviewCommon(View view, unsigned int n);
	Signs getMsgPrepareCommon(View view, unsigned int n);
	Signs getMsgPrecommitCommon(View view, unsigned int n);
	Signs getMsgCommitCommon(View view, unsigned int n);

	// Find the justification of the highest message
	Justification findHighestMsgNewviewCommon(View view);

	// Find the first message
	MsgLdrprepareCommon firstMsgLdrprepareCommon(View view);
	MsgPrepareCommon firstMsgPrepareCommon(View view);
	MsgPrecommitCommon firstMsgPrecommitCommon(View view);
	MsgCommitCommon firstMsgCommitCommon(View view);

	// Fast ResiBFT
	// Return the number of signatures
	unsigned int storeMsgNewviewFast(MsgNewviewFast msgNewview);
	unsigned int storeMsgLdrprepareFast(MsgLdrprepareFast msgLdrprepare);
	unsigned int storeMsgPrepareFast(MsgPrepareFast msgPrepare);
	unsigned int storeMsgPrecommitFast(MsgPrecommitFast msgPrecommit);
	unsigned int storeMsgValidationFast(MsgValidationFast msgValidation);

	// Collect [n] signatures of the messages
	bool checkMsgPrepareFast(View view, unsigned int n);
	bool checkMsgPrecommitFast(View view, unsigned int n);

	std::set<MsgNewviewFast> getMsgNewviewFast(View view, unsigned int n);
	Signs getMsgPrepareFast(View view, unsigned int n);
	Signs getMsgPrecommitFast(View view, unsigned int n);

	Signs getMsgPrepareFastAll(View view, unsigned int n);
	Signs getMsgPrecommitFastAll(View view, unsigned int n);

	// Find the justification of the highest message
	Justification findHighestMsgNewviewFast(View view);

	// Find the first message
	MsgLdrprepareFast firstMsgLdrprepareFast(View view);
	MsgPrepareFast firstMsgPrepareFast(View view);
	MsgPrecommitFast firstMsgPrecommitFast(View view);

	// Fast2common ResiBFT
	// Return the number of signatures
	unsigned int storeMsgLdrprepareFast2Common(MsgLdrprepareFast2Common msgLdrprepare);
	unsigned int storeMsgPrepareFast2Common(MsgPrepareFast2Common msgPrepare);
	unsigned int storeMsgPrecommitFast2Common(MsgPrecommitFast2Common msgPrecommit);
	unsigned int storeMsgCommitFast2Common(MsgCommitFast2Common msgCommit);

	// Collect [n] signatures of the messages
	Signs getMsgPrepareFast2Common(View view, unsigned int n);
	Signs getMsgPrecommitFast2Common(View view, unsigned int n);
	Signs getMsgCommitFast2Common(View view, unsigned int n);

	// Find the first message
	MsgLdrprepareFast2Common firstMsgLdrprepareFast2Common(View view);
	MsgPrepareFast2Common firstMsgPrepareFast2Common(View view);
	MsgPrecommitFast2Common firstMsgPrecommitFast2Common(View view);
	MsgCommitFast2Common firstMsgCommitFast2Common(View view);

	// Print
	std::string toPrint();
};

#endif
