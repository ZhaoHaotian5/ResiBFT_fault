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

	// Print
	std::string toPrint();
};

#endif
