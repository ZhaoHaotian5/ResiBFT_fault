#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "salticidae/msg.h"
#include "salticidae/stream.h"
#include "Accumulator.h"
#include "Committee.h"
#include "ProposalCommon.h"
#include "ProposalFast.h"
#include "RoundData.h"
#include "Sign.h"
#include "Signs.h"
#include "Transaction.h"
#include "Validations.h"

// Client messages
struct MsgTransaction
{
	static const uint8_t opcode = HEADER_TRANSACTION;
	salticidae::DataStream serialized;
	Transaction transaction;
	Sign sign;

	MsgTransaction(const Transaction &transaction, const Sign &sign) : transaction(transaction), sign(sign) { serialized << transaction << sign; }
	MsgTransaction(salticidae::DataStream &&data) { data >> transaction >> sign; }

	void serialize(salticidae::DataStream &data) const
	{
		data << transaction << sign;
	}

	unsigned int sizeMsg()
	{
		return (sizeof(Transaction) + sizeof(Sign));
	}

	std::string toPrint()
	{
		std::string text = "";
		text += "MSGTRANSACTION[";
		text += transaction.toPrint();
		text += ",";
		text += sign.toPrint();
		text += "]";
		return text;
	}

	bool operator<(const MsgTransaction &data) const
	{
		if (sign < data.sign)
		{
			return true;
		}
		return false;
	}
};

struct MsgStart
{
	static const uint8_t opcode = HEADER_START;
	salticidae::DataStream serialized;
	ClientID clientId;
	Sign sign;

	MsgStart(const ClientID &clientId, const Sign &sign) : clientId(clientId), sign(sign) { serialized << clientId << sign; }
	MsgStart(salticidae::DataStream &&data) { data >> clientId >> sign; }

	void serialize(salticidae::DataStream &data) const
	{
		data << clientId << sign;
	}

	unsigned int sizeMsg()
	{
		return (sizeof(ClientID) + sizeof(Sign));
	}

	std::string toPrint()
	{
		std::string text = "";
		text += "MSGSTART[";
		text += std::to_string(clientId);
		text += ",";
		text += sign.toPrint();
		text += "]";
		return text;
	}

	bool operator<(const MsgStart &data) const
	{
		if (clientId < data.clientId)
		{
			return true;
		}
		return false;
	}
};

struct MsgReply
{
	static const uint8_t opcode = HEADER_REPLY;
	salticidae::DataStream serialized;
	unsigned int reply;

	MsgReply(const unsigned int &reply) : reply(reply) { serialized << reply; }
	MsgReply(salticidae::DataStream &&data) { data >> reply; }

	void serialize(salticidae::DataStream &data) const
	{
		data << reply;
	}

	unsigned int sizeMsg()
	{
		return (sizeof(unsigned int));
	}

	std::string toPrint()
	{
		std::string text = "";
		text += "MSGREPLY[";
		text += std::to_string(reply);
		text += "]";
		return text;
	}

	bool operator<(const MsgReply &data) const
	{
		if (reply < data.reply)
		{
			return true;
		}
		return false;
	}
};

// Common ResiBFT
struct MsgNewviewCommon
{
	static const uint8_t opcode = HEADER_NEWVIEW_RESIBFT_COMMON;
	salticidae::DataStream serialized;
	RoundData roundData;
	Signs signs;

	MsgNewviewCommon(const RoundData &roundData, const Signs &signs) : roundData(roundData), signs(signs) { serialized << roundData << signs; }
	MsgNewviewCommon(salticidae::DataStream &&data) { data >> roundData >> signs; }

	void serialize(salticidae::DataStream &data) const
	{
		data << roundData << signs;
	}

	unsigned int sizeMsg()
	{
		return (sizeof(RoundData) + sizeof(Signs));
	}

	std::string toPrint()
	{
		std::string text = "";
		text += "RESIBFT_COMMON_MSGNEWVIEW[";
		text += roundData.toPrint();
		text += ",";
		text += signs.toPrint();
		text += "]";
		return text;
	}

	bool operator<(const MsgNewviewCommon &data) const
	{
		if (signs < data.signs)
		{
			return true;
		}
		return false;
	}
};

struct MsgLdrprepareCommon
{
	static const uint8_t opcode = HEADER_LDRPREPARE_RESIBFT_COMMON;
	salticidae::DataStream serialized;
	ProposalCommon proposalCommon;
	Committee committee;
	Signs signs;

	MsgLdrprepareCommon(const ProposalCommon &proposalCommon, const Committee &committee, const Signs &signs) : proposalCommon(proposalCommon), committee(committee), signs(signs) { serialized << proposalCommon << committee << signs; }
	MsgLdrprepareCommon(salticidae::DataStream &&data) { data >> proposalCommon >> committee >> signs; }

	void serialize(salticidae::DataStream &data) const
	{
		data << proposalCommon << committee << signs;
	}

	unsigned int sizeMsg()
	{
		return (sizeof(ProposalCommon) + sizeof(Committee) + sizeof(Signs));
	}

	std::string toPrint()
	{
		std::string text = "";
		text += "RESIBFT_COMMON_MSGLDRPREPARE[";
		text += proposalCommon.toPrint();
		text += ",";
		text += committee.toPrint();
		text += ",";
		text += signs.toPrint();
		text += "]";
		return text;
	}

	bool operator<(const MsgLdrprepareCommon &data) const
	{
		if (signs < data.signs)
		{
			return true;
		}
		return false;
	}
};

struct MsgPrepareCommon
{
	static const uint8_t opcode = HEADER_PREPARE_RESIBFT_COMMON;
	salticidae::DataStream serialized;
	RoundData roundData;
	Signs signs;

	MsgPrepareCommon(const RoundData &roundData, const Signs &signs) : roundData(roundData), signs(signs) { serialized << roundData << signs; }
	MsgPrepareCommon(salticidae::DataStream &&data) { data >> roundData >> signs; }

	void serialize(salticidae::DataStream &data) const
	{
		data << roundData << signs;
	}

	unsigned int sizeMsg()
	{
		return (sizeof(RoundData) + sizeof(Signs));
	}

	std::string toPrint()
	{
		std::string text = "";
		text += "RESIBFT_COMMON_MSGPREPARE[";
		text += roundData.toPrint();
		text += ",";
		text += signs.toPrint();
		text += "]";
		return text;
	}

	bool operator<(const MsgPrepareCommon &data) const
	{
		if (signs < data.signs)
		{
			return true;
		}
		return false;
	}
};

struct MsgPrecommitCommon
{
	static const uint8_t opcode = HEADER_PRECOMMIT_RESIBFT_COMMON;
	salticidae::DataStream serialized;
	RoundData roundData;
	Signs signs;

	MsgPrecommitCommon(const RoundData &roundData, const Signs &signs) : roundData(roundData), signs(signs) { serialized << roundData << signs; }
	MsgPrecommitCommon(salticidae::DataStream &&data) { data >> roundData >> signs; }

	void serialize(salticidae::DataStream &data) const
	{
		data << roundData << signs;
	}

	unsigned int sizeMsg()
	{
		return (sizeof(RoundData) + sizeof(Signs));
	}

	std::string toPrint()
	{
		std::string text = "";
		text += "RESIBFT_COMMON_MSGPRECOMMIT[";
		text += roundData.toPrint();
		text += ",";
		text += signs.toPrint();
		text += "]";
		return text;
	}

	bool operator<(const MsgPrecommitCommon &data) const
	{
		if (signs < data.signs)
		{
			return true;
		}
		return false;
	}
};

struct MsgCommitCommon
{
	static const uint8_t opcode = HEADER_COMMIT_RESIBFT_COMMON;
	salticidae::DataStream serialized;
	RoundData roundData;
	Signs signs;

	MsgCommitCommon(const RoundData &roundData, const Signs &signs) : roundData(roundData), signs(signs) { serialized << roundData << signs; }
	MsgCommitCommon(salticidae::DataStream &&data) { data >> roundData >> signs; }

	void serialize(salticidae::DataStream &data) const
	{
		data << roundData << signs;
	}

	unsigned int sizeMsg()
	{
		return (sizeof(RoundData) + sizeof(Signs));
	}

	std::string toPrint()
	{
		std::string text = "";
		text += "RESIBFT_COMMON_MSGCOMMIT[";
		text += roundData.toPrint();
		text += ",";
		text += signs.toPrint();
		text += "]";
		return text;
	}

	bool operator<(const MsgCommitCommon &data) const
	{
		if (signs < data.signs)
		{
			return true;
		}
		return false;
	}
};

// Fast ResiBFT
struct MsgNewviewFast
{
	static const uint8_t opcode = HEADER_NEWVIEW_RESIBFT_FAST;
	salticidae::DataStream serialized;
	bool isFail;
	RoundData roundData;
	Validation validation;
	Signs signs;

	MsgNewviewFast(const RoundData &roundData, const Validation &validation, const Signs &signs) : isFail(false), roundData(roundData), validation(validation), signs(signs) { serialized << isFail << roundData << validation << signs; }
	MsgNewviewFast(const bool &isFail, const RoundData &roundData, const Validation &validation, const Signs &signs) : isFail(isFail), roundData(roundData), validation(validation), signs(signs) { serialized << isFail << roundData << validation << signs; }
	MsgNewviewFast(salticidae::DataStream &&data) { data >> isFail >> roundData >> validation >> signs; }

	void serialize(salticidae::DataStream &data) const
	{
		data << isFail << roundData << validation << signs;
	}

	unsigned int sizeMsg()
	{
		return (sizeof(bool) + sizeof(RoundData) + sizeof(Validation) + sizeof(Signs));
	}

	std::string toPrint()
	{
		std::string textFail = "";
		if (isFail)
		{
			textFail = "FAIL";
		}
		else
		{
			textFail = "CORR";
		}
		std::string text = "";
		text += "RESIBFT_FAST_MSGNEWVIEW[";
		text += textFail;
		text += ",";
		text += roundData.toPrint();
		text += ",";
		text += validation.toPrint();
		text += ",";
		text += signs.toPrint();
		text += "]";
		return text;
	}

	bool operator<(const MsgNewviewFast &data) const
	{
		if (signs < data.signs)
		{
			return true;
		}
		return false;
	}
};

struct MsgLdrprepareFast
{
	static const uint8_t opcode = HEADER_LDRPREPARE_RESIBFT_FAST;
	salticidae::DataStream serialized;
	ProposalFast proposalFast;
	Validations validations;
	Signs signs;

	MsgLdrprepareFast(const ProposalFast &proposalFast, const Validations &validations, const Signs &signs) : proposalFast(proposalFast), validations(validations), signs(signs) { serialized << proposalFast << validations << signs; }
	MsgLdrprepareFast(salticidae::DataStream &&data) { data >> proposalFast >> validations >> signs; }

	void serialize(salticidae::DataStream &data) const
	{
		data << proposalFast << validations << signs;
	}

	unsigned int sizeMsg()
	{
		return (sizeof(ProposalFast) + sizeof(Validations) + sizeof(Signs));
	}

	std::string toPrint()
	{
		std::string text = "";
		text += "RESIBFT_FAST_MSGLDRPREPARE[";
		text += proposalFast.toPrint();
		text += ",";
		text += validations.toPrint();
		text += ",";
		text += signs.toPrint();
		text += "]";
		return text;
	}

	bool operator<(const MsgLdrprepareFast &data) const
	{
		if (signs < data.signs)
		{
			return true;
		}
		return false;
	}
};

struct MsgPrepareFast
{
	static const uint8_t opcode = HEADER_PREPARE_RESIBFT_FAST;
	salticidae::DataStream serialized;
	bool isFail;
	RoundData roundData;
	Signs signs;

	MsgPrepareFast(const RoundData &roundData, const Signs &signs) : isFail(false), roundData(roundData), signs(signs) { serialized << isFail << roundData << signs; }
	MsgPrepareFast(const bool &isFail, const RoundData &roundData, const Signs &signs) : isFail(isFail), roundData(roundData), signs(signs) { serialized << isFail << roundData << signs; }
	MsgPrepareFast(salticidae::DataStream &&data) { data >> isFail >> roundData >> signs; }

	void serialize(salticidae::DataStream &data) const
	{
		data << isFail << roundData << signs;
	}

	unsigned int sizeMsg()
	{
		return (sizeof(bool) + sizeof(RoundData) + sizeof(Signs));
	}

	std::string toPrint()
	{
		std::string textFail = "";
		if (isFail)
		{
			textFail = "FAIL";
		}
		else
		{
			textFail = "CORR";
		}
		std::string text = "";
		text += "RESIBFT_FAST_MSGPREPARE[";
		text += textFail;
		text += ",";
		text += roundData.toPrint();
		text += ",";
		text += signs.toPrint();
		text += "]";
		return text;
	}

	bool operator<(const MsgPrepareFast &data) const
	{
		if (signs < data.signs)
		{
			return true;
		}
		return false;
	}
};

struct MsgPrecommitFast
{
	static const uint8_t opcode = HEADER_PRECOMMIT_RESIBFT_FAST;
	salticidae::DataStream serialized;
	bool isFail;
	RoundData roundData;
	Signs signs;

	MsgPrecommitFast(const RoundData &roundData, const Signs &signs) : isFail(false), roundData(roundData), signs(signs) { serialized << isFail << roundData << signs; }
	MsgPrecommitFast(const bool &isFail, const RoundData &roundData, const Signs &signs) : isFail(isFail), roundData(roundData), signs(signs) { serialized << isFail << roundData << signs; }
	MsgPrecommitFast(salticidae::DataStream &&data) { data >> isFail >> roundData >> signs; }

	void serialize(salticidae::DataStream &data) const
	{
		data << isFail << roundData << signs;
	}

	unsigned int sizeMsg()
	{
		return (sizeof(bool) + sizeof(RoundData) + sizeof(Signs));
	}

	std::string toPrint()
	{
		std::string textFail = "";
		if (isFail)
		{
			textFail = "FAIL";
		}
		else
		{
			textFail = "CORR";
		}
		std::string text = "";
		text += "RESIBFT_FAST_MSGPRECOMMIT[";
		text += textFail;
		text += ",";
		text += roundData.toPrint();
		text += ",";
		text += signs.toPrint();
		text += "]";
		return text;
	}

	bool operator<(const MsgPrecommitFast &data) const
	{
		if (signs < data.signs)
		{
			return true;
		}
		return false;
	}
};

struct MsgValidationFast
{
	static const uint8_t opcode = HEADER_VALIDATION_RESIBFT_FAST;
	salticidae::DataStream serialized;
	Block block;
	Validations validations;
	RoundData roundData;
	Signs signs;

	MsgValidationFast(const Block &block, const Validations &validations, const RoundData &roundData, const Signs &signs) : block(block), validations(validations), roundData(roundData), signs(signs) { serialized << block << validations << roundData << signs; }
	MsgValidationFast(salticidae::DataStream &&data) { data >> block >> validations >> roundData >> signs; }

	void serialize(salticidae::DataStream &data) const
	{
		data << block << validations << roundData << signs;
	}

	unsigned int sizeMsg()
	{
		return (sizeof(Block) + sizeof(Validations) + sizeof(RoundData) + sizeof(Signs));
	}

	std::string toPrint()
	{
		std::string text = "";
		text += "RESIBFT_FAST_MSGVALIDATION[";
		text += block.toPrint();
		text += ",";
		text += validations.toPrint();
		text += ",";
		text += roundData.toPrint();
		text += ",";
		text += signs.toPrint();
		text += "]";
		return text;
	}

	bool operator<(const MsgValidationFast &data) const
	{
		if (signs < data.signs)
		{
			return true;
		}
		return false;
	}
};

// Fast2common 
struct MsgLdrprepareFast2Common
{
	static const uint8_t opcode = HEADER_LDRPREPARE_RESIBFT_FAST2COMMON;
	salticidae::DataStream serialized;
	ProposalCommon proposalCommon;
	Committee committee;
	Signs signs;

	MsgLdrprepareFast2Common(const ProposalCommon &proposalCommon, const Committee &committee, const Signs &signs) : proposalCommon(proposalCommon), committee(committee), signs(signs) { serialized << proposalCommon << committee << signs; }
	MsgLdrprepareFast2Common(salticidae::DataStream &&data) { data >> proposalCommon >> committee >> signs; }

	void serialize(salticidae::DataStream &data) const
	{
		data << proposalCommon << committee << signs;
	}

	unsigned int sizeMsg()
	{
		return (sizeof(ProposalCommon) + sizeof(Committee) + sizeof(Signs));
	}

	std::string toPrint()
	{
		std::string text = "";
		text += "RESIBFT_FAST2COMMON_MSGLDRPREPARE[";
		text += proposalCommon.toPrint();
		text += ",";
		text += committee.toPrint();
		text += ",";
		text += signs.toPrint();
		text += "]";
		return text;
	}

	bool operator<(const MsgLdrprepareFast2Common &data) const
	{
		if (signs < data.signs)
		{
			return true;
		}
		return false;
	}
};

struct MsgPrepareFast2Common
{
	static const uint8_t opcode = HEADER_PREPARE_RESIBFT_FAST2COMMON;
	salticidae::DataStream serialized;
	RoundData roundData;
	Signs signs;

	MsgPrepareFast2Common(const RoundData &roundData, const Signs &signs) : roundData(roundData), signs(signs) { serialized << roundData << signs; }
	MsgPrepareFast2Common(salticidae::DataStream &&data) { data >> roundData >> signs; }

	void serialize(salticidae::DataStream &data) const
	{
		data << roundData << signs;
	}

	unsigned int sizeMsg()
	{
		return (sizeof(RoundData) + sizeof(Signs));
	}

	std::string toPrint()
	{
		std::string text = "";
		text += "RESIBFT_FAST2COMMON_MSGPREPARE[";
		text += roundData.toPrint();
		text += ",";
		text += signs.toPrint();
		text += "]";
		return text;
	}

	bool operator<(const MsgPrepareFast2Common &data) const
	{
		if (signs < data.signs)
		{
			return true;
		}
		return false;
	}
};

struct MsgPrecommitFast2Common
{
	static const uint8_t opcode = HEADER_PRECOMMIT_RESIBFT_FAST2COMMON;
	salticidae::DataStream serialized;
	RoundData roundData;
	Signs signs;

	MsgPrecommitFast2Common(const RoundData &roundData, const Signs &signs) : roundData(roundData), signs(signs) { serialized << roundData << signs; }
	MsgPrecommitFast2Common(salticidae::DataStream &&data) { data >> roundData >> signs; }

	void serialize(salticidae::DataStream &data) const
	{
		data << roundData << signs;
	}

	unsigned int sizeMsg()
	{
		return (sizeof(RoundData) + sizeof(Signs));
	}

	std::string toPrint()
	{
		std::string text = "";
		text += "RESIBFT_FAST2COMMON_MSGPRECOMMIT[";
		text += roundData.toPrint();
		text += ",";
		text += signs.toPrint();
		text += "]";
		return text;
	}

	bool operator<(const MsgPrecommitFast2Common &data) const
	{
		if (signs < data.signs)
		{
			return true;
		}
		return false;
	}
};

struct MsgCommitFast2Common
{
	static const uint8_t opcode = HEADER_COMMIT_RESIBFT_FAST2COMMON;
	salticidae::DataStream serialized;
	RoundData roundData;
	Signs signs;

	MsgCommitFast2Common(const RoundData &roundData, const Signs &signs) : roundData(roundData), signs(signs) { serialized << roundData << signs; }
	MsgCommitFast2Common(salticidae::DataStream &&data) { data >> roundData >> signs; }

	void serialize(salticidae::DataStream &data) const
	{
		data << roundData << signs;
	}

	unsigned int sizeMsg()
	{
		return (sizeof(RoundData) + sizeof(Signs));
	}

	std::string toPrint()
	{
		std::string text = "";
		text += "RESIBFT_FAST2COMMON_MSGCOMMIT[";
		text += roundData.toPrint();
		text += ",";
		text += signs.toPrint();
		text += "]";
		return text;
	}

	bool operator<(const MsgCommitFast2Common &data) const
	{
		if (signs < data.signs)
		{
			return true;
		}
		return false;
	}
};

#endif