#include "Log.h"

Log::Log() {}

// Common ResiBFT
bool MsgNewviewCommonFrom(std::set<MsgNewviewCommon> msgNewviews, std::set<ReplicaID> signers)
{
	for (std::set<MsgNewviewCommon>::iterator itMsg = msgNewviews.begin(); itMsg != msgNewviews.end(); itMsg++)
	{
		MsgNewviewCommon msgNewview = *itMsg;
		std::set<ReplicaID> allSigners = msgNewview.signs.getSigners();
		for (std::set<ReplicaID>::iterator itSigner = allSigners.begin(); itSigner != allSigners.end(); itSigner++)
		{
			signers.erase(*itSigner);
			if (signers.empty())
			{
				return true;
			}
		}
	}
	return false;
}

unsigned int Log::storeMsgNewviewCommon(MsgNewviewCommon msgNewview)
{
	RoundData roundData_MsgNewview = msgNewview.roundData;
	View proposeView_MsgNewview = roundData_MsgNewview.getProposeView();
	std::set<ReplicaID> signers = msgNewview.signs.getSigners();
	std::map<View, std::set<MsgNewviewCommon>>::iterator itView = this->newviewsCommon.find(proposeView_MsgNewview);
	if (itView != this->newviewsCommon.end())
	{
		std::set<MsgNewviewCommon> msgNewviews = itView->second;
		if (!MsgNewviewCommonFrom(msgNewviews, signers))
		{
			msgNewviews.insert(msgNewview);
			this->newviewsCommon[proposeView_MsgNewview] = msgNewviews;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Updated entry for msgNewview in common path in view " << proposeView_MsgNewview << " and the number of msgNewview is: " << msgNewviews.size() << COLOUR_NORMAL << std::endl;
			}
			return msgNewviews.size();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		std::set<MsgNewviewCommon> msgNewviews = {msgNewview};
		this->newviewsCommon[proposeView_MsgNewview] = msgNewviews;
		if (DEBUG_LOG)
		{
			std::cout << COLOUR_GREEN << "No entry for msgNewview in common path in view " << proposeView_MsgNewview << " and the number of msgNewview is: 1" << COLOUR_NORMAL << std::endl;
		}
		return 1;
	}
}

bool MsgLdrprepareCommonFrom(std::set<MsgLdrprepareCommon> msgLdrprepares, std::set<ReplicaID> signers)
{
	for (std::set<MsgLdrprepareCommon>::iterator itMsg = msgLdrprepares.begin(); itMsg != msgLdrprepares.end(); itMsg++)
	{
		MsgLdrprepareCommon msgLdrprepare = *itMsg;
		std::set<ReplicaID> allSigners = msgLdrprepare.signs.getSigners();
		for (std::set<ReplicaID>::iterator itSigner = allSigners.begin(); itSigner != allSigners.end(); itSigner++)
		{
			signers.erase(*itSigner);
			if (signers.empty())
			{
				return true;
			}
		}
	}
	return false;
}

unsigned int Log::storeMsgLdrprepareCommon(MsgLdrprepareCommon msgLdrprepare)
{
	ProposalCommon proposal_MsgLdrprepare = msgLdrprepare.proposalCommon;
	View proposeView_MsgLdrprepare = proposal_MsgLdrprepare.getJustification().getRoundData().getProposeView();
	std::set<ReplicaID> signers = msgLdrprepare.signs.getSigners();
	std::map<View, std::set<MsgLdrprepareCommon>>::iterator itView = this->ldrpreparesCommon.find(proposeView_MsgLdrprepare);
	if (itView != this->ldrpreparesCommon.end())
	{
		std::set<MsgLdrprepareCommon> msgLdrprepares = itView->second;
		if (!MsgLdrprepareCommonFrom(msgLdrprepares, signers))
		{
			msgLdrprepares.insert(msgLdrprepare);
			this->ldrpreparesCommon[proposeView_MsgLdrprepare] = msgLdrprepares;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Updated entry for msgLdrprepare in common path in view " << proposeView_MsgLdrprepare << " and the number of msgLdrprepare is: " << msgLdrprepares.size() << COLOUR_NORMAL << std::endl;
			}
			return msgLdrprepares.size();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		std::set<MsgLdrprepareCommon> msgLdrprepares = {msgLdrprepare};
		this->ldrpreparesCommon[proposeView_MsgLdrprepare] = msgLdrprepares;
		if (DEBUG_LOG)
		{
			std::cout << COLOUR_GREEN << "No entry for msgLdrprepare in common path in view " << proposeView_MsgLdrprepare << " and the number of msgLdrprepare is: 1" << COLOUR_NORMAL << std::endl;
		}
		return 1;
	}
}

bool MsgPrepareCommonFrom(std::set<MsgPrepareCommon> msgPrepares, std::set<ReplicaID> signers)
{
	for (std::set<MsgPrepareCommon>::iterator itMsg = msgPrepares.begin(); itMsg != msgPrepares.end(); itMsg++)
	{
		MsgPrepareCommon msgPrepare = *itMsg;
		std::set<ReplicaID> allSigners = msgPrepare.signs.getSigners();
		for (std::set<ReplicaID>::iterator itSigner = allSigners.begin(); itSigner != allSigners.end(); itSigner++)
		{
			signers.erase(*itSigner);
			if (signers.empty())
			{
				return true;
			}
		}
	}
	return false;
}

unsigned int Log::storeMsgPrepareCommon(MsgPrepareCommon msgPrepare)
{
	RoundData roundData_MsgPrepare = msgPrepare.roundData;
	View proposeView_MsgPrepare = roundData_MsgPrepare.getProposeView();
	std::set<ReplicaID> signers = msgPrepare.signs.getSigners();
	std::map<View, std::set<MsgPrepareCommon>>::iterator itView = this->preparesCommon.find(proposeView_MsgPrepare);
	if (itView != this->preparesCommon.end())
	{
		std::set<MsgPrepareCommon> msgPrepares = itView->second;
		if (!MsgPrepareCommonFrom(msgPrepares, signers))
		{
			msgPrepares.insert(msgPrepare);
			this->preparesCommon[proposeView_MsgPrepare] = msgPrepares;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Updated entry for msgPrepare in common path in view " << proposeView_MsgPrepare << " and the number of msgPrepare is: " << msgPrepares.size() << COLOUR_NORMAL << std::endl;
			}
			return msgPrepares.size();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		std::set<MsgPrepareCommon> msgPrepares = {msgPrepare};
		this->preparesCommon[proposeView_MsgPrepare] = msgPrepares;
		if (DEBUG_LOG)
		{
			std::cout << COLOUR_GREEN << "No entry for msgPrepare in common path in view " << proposeView_MsgPrepare << " and the number of msgPrepare is: 1" << COLOUR_NORMAL << std::endl;
		}
		return 1;
	}
}

bool MsgPrecommitCommonFrom(std::set<MsgPrecommitCommon> msgPrecommits, std::set<ReplicaID> signers)
{
	for (std::set<MsgPrecommitCommon>::iterator itMsg = msgPrecommits.begin(); itMsg != msgPrecommits.end(); itMsg++)
	{
		MsgPrecommitCommon msgPrecommit = *itMsg;
		std::set<ReplicaID> allSigners = msgPrecommit.signs.getSigners();
		for (std::set<ReplicaID>::iterator itSigner = allSigners.begin(); itSigner != allSigners.end(); itSigner++)
		{
			signers.erase(*itSigner);
			if (signers.empty())
			{
				return true;
			}
		}
	}
	return false;
}

unsigned int Log::storeMsgPrecommitCommon(MsgPrecommitCommon msgPrecommit)
{
	RoundData roundData_MsgPrecommit = msgPrecommit.roundData;
	View proposeView_MsgPrecommit = roundData_MsgPrecommit.getProposeView();
	std::set<ReplicaID> signers = msgPrecommit.signs.getSigners();
	std::map<View, std::set<MsgPrecommitCommon>>::iterator itView = this->precommitsCommon.find(proposeView_MsgPrecommit);
	if (itView != this->precommitsCommon.end())
	{
		std::set<MsgPrecommitCommon> msgPrecommits = itView->second;
		if (!MsgPrecommitCommonFrom(msgPrecommits, signers))
		{
			msgPrecommits.insert(msgPrecommit);
			this->precommitsCommon[proposeView_MsgPrecommit] = msgPrecommits;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Updated entry for msgPrecommit in common path in view " << proposeView_MsgPrecommit << " and the number of msgPrecommit is: " << msgPrecommits.size() << COLOUR_NORMAL << std::endl;
			}
			return msgPrecommits.size();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		std::set<MsgPrecommitCommon> msgPrecommits = {msgPrecommit};
		this->precommitsCommon[proposeView_MsgPrecommit] = msgPrecommits;
		if (DEBUG_LOG)
		{
			std::cout << COLOUR_GREEN << "No entry for msgPrecommit in common path in view " << proposeView_MsgPrecommit << " and the number of msgPrecommit is: 1" << COLOUR_NORMAL << std::endl;
		}
		return 1;
	}
}

bool MsgCommitCommonFrom(std::set<MsgCommitCommon> msgCommits, std::set<ReplicaID> signers)
{
	for (std::set<MsgCommitCommon>::iterator itMsg = msgCommits.begin(); itMsg != msgCommits.end(); itMsg++)
	{
		MsgCommitCommon msgCommit = *itMsg;
		std::set<ReplicaID> allSigners = msgCommit.signs.getSigners();
		for (std::set<ReplicaID>::iterator itSigner = allSigners.begin(); itSigner != allSigners.end(); itSigner++)
		{
			signers.erase(*itSigner);
			if (signers.empty())
			{
				return true;
			}
		}
	}
	return false;
}

unsigned int Log::storeMsgCommitCommon(MsgCommitCommon msgCommit)
{
	RoundData roundData_MsgCommit = msgCommit.roundData;
	View proposeView_MsgCommit = roundData_MsgCommit.getProposeView();
	std::set<ReplicaID> signers = msgCommit.signs.getSigners();
	std::map<View, std::set<MsgCommitCommon>>::iterator itView = this->commitsCommon.find(proposeView_MsgCommit);
	if (itView != this->commitsCommon.end())
	{
		std::set<MsgCommitCommon> msgCommits = itView->second;
		if (!MsgCommitCommonFrom(msgCommits, signers))
		{
			msgCommits.insert(msgCommit);
			this->commitsCommon[proposeView_MsgCommit] = msgCommits;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Updated entry for msgCommit in common path in view " << proposeView_MsgCommit << " and the number of msgCommit is: " << msgCommits.size() << COLOUR_NORMAL << std::endl;
			}
			return msgCommits.size();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		std::set<MsgCommitCommon> msgCommits = {msgCommit};
		this->commitsCommon[proposeView_MsgCommit] = msgCommits;
		if (DEBUG_LOG)
		{
			std::cout << COLOUR_GREEN << "No entry for msgCommit in common path in view " << proposeView_MsgCommit << " and the number of msgCommit is: 1" << COLOUR_NORMAL << std::endl;
		}
		return 1;
	}
}

Signs Log::getMsgNewviewCommon(View view, unsigned int n)
{
	Signs signs;
	std::map<View, std::set<MsgNewviewCommon>>::iterator itView = this->newviewsCommon.find(view);
	if (itView != this->newviewsCommon.end())
	{
		std::set<MsgNewviewCommon> msgNewviews = itView->second;
		for (std::set<MsgNewviewCommon>::iterator itMsg = msgNewviews.begin(); signs.getSize() < n && itMsg != msgNewviews.end(); itMsg++)
		{
			MsgNewviewCommon msgNewview = *itMsg;
			Signs signs_MsgNewview = msgNewview.signs;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Adding signatures: " << signs_MsgNewview.toPrint() << COLOUR_NORMAL << std::endl;
			}
			signs.addUpto(signs_MsgNewview, n);
		}
	}
	if (DEBUG_LOG)
	{
		std::cout << COLOUR_GREEN << "Log signatures: " << signs.toPrint() << COLOUR_NORMAL << std::endl;
	}
	return signs;
}

Signs Log::getMsgPrepareCommon(View view, unsigned int n)
{
	Signs signs;
	std::map<View, std::set<MsgPrepareCommon>>::iterator itView = this->preparesCommon.find(view);
	if (itView != this->preparesCommon.end())
	{
		std::set<MsgPrepareCommon> msgPrepares = itView->second;
		for (std::set<MsgPrepareCommon>::iterator itMsg = msgPrepares.begin(); signs.getSize() < n && itMsg != msgPrepares.end(); itMsg++)
		{
			MsgPrepareCommon msgPrepare = *itMsg;
			Signs signs_MsgPrepare = msgPrepare.signs;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Adding signatures: " << signs_MsgPrepare.toPrint() << COLOUR_NORMAL << std::endl;
			}
			signs.addUpto(signs_MsgPrepare, n);
		}
	}
	if (DEBUG_LOG)
	{
		std::cout << COLOUR_GREEN << "Log signatures: " << signs.toPrint() << COLOUR_NORMAL << std::endl;
	}
	return signs;
}

Signs Log::getMsgPrecommitCommon(View view, unsigned int n)
{
	Signs signs;
	std::map<View, std::set<MsgPrecommitCommon>>::iterator itView = this->precommitsCommon.find(view);
	if (itView != this->precommitsCommon.end())
	{
		std::set<MsgPrecommitCommon> msgPrecommits = itView->second;
		for (std::set<MsgPrecommitCommon>::iterator itMsg = msgPrecommits.begin(); signs.getSize() < n && itMsg != msgPrecommits.end(); itMsg++)
		{
			MsgPrecommitCommon msgPrecommit = *itMsg;
			Signs signs_MsgPrecommit = msgPrecommit.signs;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Adding signatures: " << signs_MsgPrecommit.toPrint() << COLOUR_NORMAL << std::endl;
			}
			signs.addUpto(signs_MsgPrecommit, n);
		}
	}
	if (DEBUG_LOG)
	{
		std::cout << COLOUR_GREEN << "Log signatures: " << signs.toPrint() << COLOUR_NORMAL << std::endl;
	}
	return signs;
}

Signs Log::getMsgCommitCommon(View view, unsigned int n)
{
	Signs signs;
	std::map<View, std::set<MsgCommitCommon>>::iterator itView = this->commitsCommon.find(view);
	if (itView != this->commitsCommon.end())
	{
		std::set<MsgCommitCommon> msgCommits = itView->second;
		for (std::set<MsgCommitCommon>::iterator itMsg = msgCommits.begin(); signs.getSize() < n && itMsg != msgCommits.end(); itMsg++)
		{
			MsgCommitCommon msgCommit = *itMsg;
			Signs signs_MsgCommit = msgCommit.signs;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Adding signatures: " << signs_MsgCommit.toPrint() << COLOUR_NORMAL << std::endl;
			}
			signs.addUpto(signs_MsgCommit, n);
		}
	}
	if (DEBUG_LOG)
	{
		std::cout << COLOUR_GREEN << "Log signatures: " << signs.toPrint() << COLOUR_NORMAL << std::endl;
	}
	return signs;
}

Justification Log::findHighestMsgNewviewCommon(View view)
{
	std::map<View, std::set<MsgNewviewCommon>>::iterator itView = this->newviewsCommon.find(view);
	Justification justification_MsgNewview = Justification();
	if (itView != this->newviewsCommon.end())
	{
		std::set<MsgNewviewCommon> msgNewviews = itView->second;
		View highView = 0;
		for (std::set<MsgNewviewCommon>::iterator itMsg = msgNewviews.begin(); itMsg != msgNewviews.end(); itMsg++)
		{
			MsgNewviewCommon msgNewview = *itMsg;
			RoundData roundData_MsgNewview = msgNewview.roundData;
			Signs signs_MsgNewview = msgNewview.signs;
			View justifyView_MsgNewview = roundData_MsgNewview.getJustifyView();
			if (justifyView_MsgNewview >= highView)
			{
				highView = justifyView_MsgNewview;
				justification_MsgNewview = Justification(roundData_MsgNewview, signs_MsgNewview);
			}
		}
	}
	return justification_MsgNewview;
}

MsgLdrprepareCommon Log::firstMsgLdrprepareCommon(View view)
{
	std::map<View, std::set<MsgLdrprepareCommon>>::iterator itView = this->ldrpreparesCommon.find(view);
	if (itView != this->ldrpreparesCommon.end())
	{
		std::set<MsgLdrprepareCommon> msgLdrprepares = itView->second;
		if (msgLdrprepares.size() > 0)
		{
			std::set<MsgLdrprepareCommon>::iterator itMsg = msgLdrprepares.begin();
			MsgLdrprepareCommon msgLdrprepare = *itMsg;
			return msgLdrprepare;
		}
	}
	ProposalCommon proposal;
	Committee committee;
	Signs signs;
	MsgLdrprepareCommon msgLdrprepare = MsgLdrprepareCommon(proposal, committee, signs);
	return msgLdrprepare;
}

MsgPrepareCommon Log::firstMsgPrepareCommon(View view)
{
	std::map<View, std::set<MsgPrepareCommon>>::iterator itView = this->preparesCommon.find(view);
	if (itView != this->preparesCommon.end())
	{
		std::set<MsgPrepareCommon> msgPrepares = itView->second;
		if (msgPrepares.size() > 0)
		{
			std::set<MsgPrepareCommon>::iterator itMsg = msgPrepares.begin();
			MsgPrepareCommon msgPrepare = *itMsg;
			return msgPrepare;
		}
	}
	RoundData roundData;
	Signs signs;
	MsgPrepareCommon msgPrepare = MsgPrepareCommon(roundData, signs);
	return msgPrepare;
}

MsgPrecommitCommon Log::firstMsgPrecommitCommon(View view)
{
	std::map<View, std::set<MsgPrecommitCommon>>::iterator itView = this->precommitsCommon.find(view);
	if (itView != this->precommitsCommon.end())
	{
		std::set<MsgPrecommitCommon> msgPrecommits = itView->second;
		if (msgPrecommits.size() > 0)
		{
			std::set<MsgPrecommitCommon>::iterator itMsg = msgPrecommits.begin();
			MsgPrecommitCommon msgPrecommit = *itMsg;
			return msgPrecommit;
		}
	}
	RoundData roundData;
	Signs signs;
	MsgPrecommitCommon msgPrecommit = MsgPrecommitCommon(roundData, signs);
	return msgPrecommit;
}

MsgCommitCommon Log::firstMsgCommitCommon(View view)
{
	std::map<View, std::set<MsgCommitCommon>>::iterator itView = this->commitsCommon.find(view);
	if (itView != this->commitsCommon.end())
	{
		std::set<MsgCommitCommon> msgCommits = itView->second;
		if (msgCommits.size() > 0)
		{
			std::set<MsgCommitCommon>::iterator itMsg = msgCommits.begin();
			MsgCommitCommon msgCommit = *itMsg;
			return msgCommit;
		}
	}
	RoundData roundData;
	Signs signs;
	MsgCommitCommon msgCommit = MsgCommitCommon(roundData, signs);
	return msgCommit;
}

// Fast ResiBFT
bool MsgNewviewFastFrom(std::set<MsgNewviewFast> msgNewviews, std::set<ReplicaID> signers)
{
	for (std::set<MsgNewviewFast>::iterator itMsg = msgNewviews.begin(); itMsg != msgNewviews.end(); itMsg++)
	{
		MsgNewviewFast msgNewview = *itMsg;
		std::set<ReplicaID> allSigners = msgNewview.signs.getSigners();
		for (std::set<ReplicaID>::iterator itSigner = allSigners.begin(); itSigner != allSigners.end(); itSigner++)
		{
			signers.erase(*itSigner);
			if (signers.empty())
			{
				return true;
			}
		}
	}
	return false;
}

unsigned int Log::storeMsgNewviewFast(MsgNewviewFast msgNewview)
{
	RoundData roundData_MsgNewview = msgNewview.roundData;
	View proposeView_MsgNewview = roundData_MsgNewview.getProposeView();
	std::set<ReplicaID> signers = msgNewview.signs.getSigners();
	std::map<View, std::set<MsgNewviewFast>>::iterator itView = this->newviewsFast.find(proposeView_MsgNewview);
	if (itView != this->newviewsFast.end())
	{
		std::set<MsgNewviewFast> msgNewviews = itView->second;
		if (!MsgNewviewFastFrom(msgNewviews, signers))
		{
			msgNewviews.insert(msgNewview);
			this->newviewsFast[proposeView_MsgNewview] = msgNewviews;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Updated entry for MsgNewview in fast path in view " << proposeView_MsgNewview << " and the number of MsgNewview is: " << msgNewviews.size() << COLOUR_NORMAL << std::endl;
			}
			return msgNewviews.size();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		std::set<MsgNewviewFast> msgNewviews = {msgNewview};
		this->newviewsFast[proposeView_MsgNewview] = msgNewviews;
		if (DEBUG_LOG)
		{
			std::cout << COLOUR_GREEN << "No entry for MsgNewview in fast path in view " << proposeView_MsgNewview << " and the number of MsgNewview is: 1" << COLOUR_NORMAL << std::endl;
		}
		return 1;
	}
}

bool MsgLdrprepareFastFrom(std::set<MsgLdrprepareFast> msgLdrprepares, std::set<ReplicaID> signers)
{
	for (std::set<MsgLdrprepareFast>::iterator itMsg = msgLdrprepares.begin(); itMsg != msgLdrprepares.end(); itMsg++)
	{
		MsgLdrprepareFast msgLdrprepare = *itMsg;
		std::set<ReplicaID> allSigners = msgLdrprepare.signs.getSigners();
		for (std::set<ReplicaID>::iterator itSigners = allSigners.begin(); itSigners != allSigners.end(); itSigners++)
		{
			signers.erase(*itSigners);
			if (signers.empty())
			{
				return true;
			}
		}
	}
	return false;
}

unsigned int Log::storeMsgLdrprepareFast(MsgLdrprepareFast msgLdrprepare)
{
	Accumulator accumulator_MsgLdrprepare = msgLdrprepare.proposalFast.getAccumulator();
	View proposeView_MsgLdrprepare = accumulator_MsgLdrprepare.getProposeView();
	std::set<ReplicaID> signers = msgLdrprepare.signs.getSigners();

	std::map<View, std::set<MsgLdrprepareFast>>::iterator itView = this->ldrpreparesFast.find(proposeView_MsgLdrprepare);
	if (itView != this->ldrpreparesFast.end())
	{
		std::set<MsgLdrprepareFast> msgLdrprepares = itView->second;
		if (!MsgLdrprepareFastFrom(msgLdrprepares, signers))
		{
			msgLdrprepares.insert(msgLdrprepare);
			this->ldrpreparesFast[proposeView_MsgLdrprepare] = msgLdrprepares;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Updated entry for MsgLdrprepare in fast path in view " << proposeView_MsgLdrprepare << " and the number of MsgLdrprepare is: " << msgLdrprepares.size() << COLOUR_NORMAL << std::endl;
			}
			return msgLdrprepares.size();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		std::set<MsgLdrprepareFast> msgLdrprepares = {msgLdrprepare};
		this->ldrpreparesFast[proposeView_MsgLdrprepare] = msgLdrprepares;
		if (DEBUG_LOG)
		{
			std::cout << COLOUR_GREEN << "No entry for MsgLdrprepare in fast path in view " << proposeView_MsgLdrprepare << " and the number of MsgLdrprepare is: 1" << COLOUR_NORMAL << std::endl;
		}
		return 1;
	}
}

bool MsgPrepareFastFrom(std::set<MsgPrepareFast> msgPrepares, std::set<ReplicaID> signers)
{
	for (std::set<MsgPrepareFast>::iterator itMsg = msgPrepares.begin(); itMsg != msgPrepares.end(); itMsg++)
	{
		MsgPrepareFast msgPrepare = *itMsg;
		std::set<ReplicaID> allSigners = msgPrepare.signs.getSigners();
		for (std::set<ReplicaID>::iterator itSigners = allSigners.begin(); itSigners != allSigners.end(); itSigners++)
		{
			signers.erase(*itSigners);
			if (signers.empty())
			{
				return true;
			}
		}
	}
	return false;
}

unsigned int Log::storeMsgPrepareFast(MsgPrepareFast msgPrepare)
{
	RoundData roundData_MsgPrepare = msgPrepare.roundData;
	View proposeView_MsgPrepare = roundData_MsgPrepare.getProposeView();
	std::set<ReplicaID> signers = msgPrepare.signs.getSigners();

	std::map<View, std::set<MsgPrepareFast>>::iterator itView = this->preparesFast.find(proposeView_MsgPrepare);
	if (itView != this->preparesFast.end())
	{
		std::set<MsgPrepareFast> msgPrepares = itView->second;
		if (!MsgPrepareFastFrom(msgPrepares, signers))
		{
			msgPrepares.insert(msgPrepare);
			this->preparesFast[proposeView_MsgPrepare] = msgPrepares;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Updated entry for MsgPrepare in fast path in view " << proposeView_MsgPrepare << " and the number of MsgPrepare is: " << msgPrepares.size() << COLOUR_NORMAL << std::endl;
			}
			return msgPrepares.size();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		std::set<MsgPrepareFast> msgPrepares = {msgPrepare};
		this->preparesFast[proposeView_MsgPrepare] = msgPrepares;
		if (DEBUG_LOG)
		{
			std::cout << COLOUR_GREEN << "No entry for MsgPrepare in fast path in view " << proposeView_MsgPrepare << " and the number of MsgPrepare is: 1" << COLOUR_NORMAL << std::endl;
		}
		return 1;
	}
}

bool MsgPrecommitFastFrom(std::set<MsgPrecommitFast> msgPrecommits, std::set<ReplicaID> signers)
{
	for (std::set<MsgPrecommitFast>::iterator itMsg = msgPrecommits.begin(); itMsg != msgPrecommits.end(); itMsg++)
	{
		MsgPrecommitFast msgPrecommit = *itMsg;
		std::set<ReplicaID> allSigners = msgPrecommit.signs.getSigners();
		for (std::set<ReplicaID>::iterator itSigners = allSigners.begin(); itSigners != allSigners.end(); itSigners++)
		{
			signers.erase(*itSigners);
			if (signers.empty())
			{
				return true;
			}
		}
	}
	return false;
}

unsigned int Log::storeMsgPrecommitFast(MsgPrecommitFast msgPrecommit)
{
	RoundData roundData_MsgPrecommit = msgPrecommit.roundData;
	View proposeView_MsgPrecommit = roundData_MsgPrecommit.getProposeView();
	std::set<ReplicaID> signers = msgPrecommit.signs.getSigners();

	std::map<View, std::set<MsgPrecommitFast>>::iterator itView = this->precommitsFast.find(proposeView_MsgPrecommit);
	if (itView != this->precommitsFast.end())
	{
		std::set<MsgPrecommitFast> msgPrecommits = itView->second;
		if (!MsgPrecommitFastFrom(msgPrecommits, signers))
		{
			msgPrecommits.insert(msgPrecommit);
			this->precommitsFast[proposeView_MsgPrecommit] = msgPrecommits;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Updated entry for MsgPrecommit in fast path in view " << proposeView_MsgPrecommit << " and the number of MsgPrecommit is: " << msgPrecommits.size() << COLOUR_NORMAL << std::endl;
			}
			return msgPrecommits.size();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		std::set<MsgPrecommitFast> msgPrecommits = {msgPrecommit};
		this->precommitsFast[proposeView_MsgPrecommit] = {msgPrecommit};
		if (DEBUG_LOG)
		{
			std::cout << COLOUR_GREEN << "No entry for MsgPrecommit in fast path in view " << proposeView_MsgPrecommit << " and the number of MsgPrecommit is: 1" << COLOUR_NORMAL << std::endl;
		}
		return 1;
	}
}

bool MsgValidationFastFrom(std::set<MsgValidationFast> msgValidations, std::set<ReplicaID> signers)
{
	for (std::set<MsgValidationFast>::iterator itMsg = msgValidations.begin(); itMsg != msgValidations.end(); itMsg++)
	{
		MsgValidationFast msgValidation = *itMsg;
		std::set<ReplicaID> allSigners = msgValidation.signs.getSigners();
		for (std::set<ReplicaID>::iterator itSigners = allSigners.begin(); itSigners != allSigners.end(); itSigners++)
		{
			signers.erase(*itSigners);
			if (signers.empty())
			{
				return true;
			}
		}
	}
	return false;
}

unsigned int Log::storeMsgValidationFast(MsgValidationFast msgValidation)
{
	RoundData roundData_MsgValidation = msgValidation.roundData;
	View proposeView_MsgValidation = roundData_MsgValidation.getProposeView();
	std::set<ReplicaID> signers = msgValidation.signs.getSigners();

	std::map<View, std::set<MsgValidationFast>>::iterator itView = this->validationsFast.find(proposeView_MsgValidation);
	if (itView != this->validationsFast.end())
	{
		std::set<MsgValidationFast> msgValidations = itView->second;
		if (!MsgValidationFastFrom(msgValidations, signers))
		{
			msgValidations.insert(msgValidation);
			this->validationsFast[proposeView_MsgValidation] = msgValidations;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Updated entry for MsgValidation in fast path in view " << proposeView_MsgValidation << " and the number of MsgValidation is: " << msgValidations.size() << COLOUR_NORMAL << std::endl;
			}
			return msgValidations.size();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		std::set<MsgValidationFast> msgValidations = {msgValidation};
		this->validationsFast[proposeView_MsgValidation] = {msgValidations};
		if (DEBUG_LOG)
		{
			std::cout << COLOUR_GREEN << "No entry for MsgValidation in fast path in view " << proposeView_MsgValidation << " and the number of MsgValidation is: 1" << COLOUR_NORMAL << std::endl;
		}
		return 1;
	}
}

std::set<MsgNewviewFast> Log::getMsgNewviewFast(View view, unsigned int n)
{
	std::set<MsgNewviewFast> msgNewview;
	std::map<View, std::set<MsgNewviewFast>>::iterator itView = this->newviewsFast.find(view);
	if (itView != this->newviewsFast.end())
	{
		std::set<MsgNewviewFast> msgNewviews = itView->second;
		for (std::set<MsgNewviewFast>::iterator itMsg = msgNewviews.begin(); msgNewview.size() < n && itMsg != msgNewviews.end(); itMsg++)
		{
			MsgNewviewFast msg = *itMsg;
			msgNewview.insert(msg);
		}
	}
	return msgNewview;
}

Signs Log::getMsgPrepareFast(View view, unsigned int n)
{
	Signs signs;
	std::map<View, std::set<MsgPrepareFast>>::iterator itView = this->preparesFast.find(view);
	if (itView != this->preparesFast.end())
	{
		std::set<MsgPrepareFast> msgPrepares = itView->second;
		for (std::set<MsgPrepareFast>::iterator itMsg = msgPrepares.begin(); signs.getSize() < n && itMsg != msgPrepares.end(); itMsg++)
		{
			MsgPrepareFast msgPrepare = *itMsg;
			Signs signs_MsgPrepare = msgPrepare.signs;
			if (!msgPrepare.isFail)
			{
				if (DEBUG_LOG)
				{
					std::cout << COLOUR_GREEN << "Adding signatures: " << signs_MsgPrepare.toPrint() << COLOUR_NORMAL << std::endl;
				}
				signs.addUpto(signs_MsgPrepare, n);
			}
		}
	}
	if (DEBUG_LOG)
	{
		std::cout << COLOUR_GREEN << "Log signatures: " << signs.toPrint() << COLOUR_NORMAL << std::endl;
	}
	return signs;
}

Signs Log::getMsgPrecommitFast(View view, unsigned int n)
{
	Signs signs;
	std::map<View, std::set<MsgPrecommitFast>>::iterator itView = this->precommitsFast.find(view);
	if (itView != this->precommitsFast.end())
	{
		std::set<MsgPrecommitFast> msgPrecommits = itView->second;
		for (std::set<MsgPrecommitFast>::iterator itMsg = msgPrecommits.begin(); signs.getSize() < n && itMsg != msgPrecommits.end(); itMsg++)
		{
			MsgPrecommitFast msgPrecommit = *itMsg;
			Signs signs_MsgPrecommit = msgPrecommit.signs;
			if (!msgPrecommit.isFail)
			{
				if (DEBUG_LOG)
				{
					std::cout << COLOUR_GREEN << "Adding signatures: " << signs_MsgPrecommit.toPrint() << COLOUR_NORMAL << std::endl;
				}
				signs.addUpto(signs_MsgPrecommit, n);
			}
		}
	}
	if (DEBUG_LOG)
	{
		std::cout << COLOUR_GREEN << "Log signatures: " << signs.toPrint() << COLOUR_NORMAL << std::endl;
	}
	return signs;
}

Signs Log::getMsgPrepareFastAll(View view, unsigned int n)
{
	Signs signs;
	std::map<View, std::set<MsgPrepareFast>>::iterator itView = this->preparesFast.find(view);
	if (itView != this->preparesFast.end())
	{
		std::set<MsgPrepareFast> msgPrepares = itView->second;
		for (std::set<MsgPrepareFast>::iterator itMsg = msgPrepares.begin(); signs.getSize() < n && itMsg != msgPrepares.end(); itMsg++)
		{
			MsgPrepareFast msgPrepare = *itMsg;
			Signs signs_MsgPrepare = msgPrepare.signs;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Adding signatures: " << signs_MsgPrepare.toPrint() << COLOUR_NORMAL << std::endl;
			}
			signs.addUpto(signs_MsgPrepare, n);
		}
	}
	if (DEBUG_LOG)
	{
		std::cout << COLOUR_GREEN << "Log signatures: " << signs.toPrint() << COLOUR_NORMAL << std::endl;
	}
	return signs;
}

Signs Log::getMsgPrecommitFastAll(View view, unsigned int n)
{
	Signs signs;
	std::map<View, std::set<MsgPrecommitFast>>::iterator itView = this->precommitsFast.find(view);
	if (itView != this->precommitsFast.end())
	{
		std::set<MsgPrecommitFast> msgPrecommits = itView->second;
		for (std::set<MsgPrecommitFast>::iterator itMsg = msgPrecommits.begin(); signs.getSize() < n && itMsg != msgPrecommits.end(); itMsg++)
		{
			MsgPrecommitFast msgPrecommit = *itMsg;
			Signs signs_MsgPrecommit = msgPrecommit.signs;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Adding signatures: " << signs_MsgPrecommit.toPrint() << COLOUR_NORMAL << std::endl;
			}
			signs.addUpto(signs_MsgPrecommit, n);
		}
	}
	if (DEBUG_LOG)
	{
		std::cout << COLOUR_GREEN << "Log signatures: " << signs.toPrint() << COLOUR_NORMAL << std::endl;
	}
	return signs;
}

Justification Log::findHighestMsgNewviewFast(View view)
{
	std::map<View, std::set<MsgNewviewFast>>::iterator itView = this->newviewsFast.find(view);
	Justification justification_MsgNewviewFast = Justification();
	if (itView != this->newviewsFast.end())
	{
		std::set<MsgNewviewFast> msgNewviewsFast = itView->second;
		View highView = 0;
		for (std::set<MsgNewviewFast>::iterator itMsg = msgNewviewsFast.begin(); itMsg != msgNewviewsFast.end(); itMsg++)
		{
			MsgNewviewFast msgNewviewFast = *itMsg;
			RoundData roundData_MsgNewviewFast = msgNewviewFast.roundData;
			Signs signs_MsgNewviewFast = msgNewviewFast.signs;
			View justifyView_MsgNewviewFast = roundData_MsgNewviewFast.getJustifyView();
			if (justifyView_MsgNewviewFast >= highView)
			{
				highView = justifyView_MsgNewviewFast;
				justification_MsgNewviewFast = Justification(roundData_MsgNewviewFast, signs_MsgNewviewFast);
			}
		}
	}
	return justification_MsgNewviewFast;
}

MsgLdrprepareFast Log::firstMsgLdrprepareFast(View view)
{
	std::map<View, std::set<MsgLdrprepareFast>>::iterator itView = this->ldrpreparesFast.find(view);
	if (itView != this->ldrpreparesFast.end())
	{
		std::set<MsgLdrprepareFast> msgLdrprepares = itView->second;
		if (msgLdrprepares.size() > 0)
		{
			std::set<MsgLdrprepareFast>::iterator itMsg = msgLdrprepares.begin();
			MsgLdrprepareFast msgLdrprepare = *itMsg;
			return msgLdrprepare;
		}
	}
	ProposalFast proposal;
	Validations validations;
	Signs signs;
	MsgLdrprepareFast msgLdrprepare = MsgLdrprepareFast(proposal, validations, signs);
	return msgLdrprepare;
}

MsgPrepareFast Log::firstMsgPrepareFast(View view)
{
	std::map<View, std::set<MsgPrepareFast>>::iterator itView = this->preparesFast.find(view);
	if (itView != this->preparesFast.end())
	{
		std::set<MsgPrepareFast> msgPrepares = itView->second;
		if (msgPrepares.size() > 0)
		{
			std::set<MsgPrepareFast>::iterator itMsg = msgPrepares.begin();
			MsgPrepareFast msgPrepare = *itMsg;
			return msgPrepare;
		}
	}
	RoundData roundData;
	Signs signs;
	MsgPrepareFast msgPrepare = MsgPrepareFast(roundData, signs);
	return msgPrepare;
}

MsgPrecommitFast Log::firstMsgPrecommitFast(View view)
{
	std::map<View, std::set<MsgPrecommitFast>>::iterator itView = this->precommitsFast.find(view);
	if (itView != this->precommitsFast.end())
	{
		std::set<MsgPrecommitFast> msgPrecommits = itView->second;
		if (msgPrecommits.size() > 0)
		{
			std::set<MsgPrecommitFast>::iterator itMsg = msgPrecommits.begin();
			MsgPrecommitFast msgPrecommit = *itMsg;
			return msgPrecommit;
		}
	}
	RoundData roundData;
	Signs signs;
	MsgPrecommitFast msgPrecommit = MsgPrecommitFast(roundData, signs);
	return msgPrecommit;
}

// Fast2common ResiBFT
bool MsgLdrprepareFast2CommonFrom(std::set<MsgLdrprepareFast2Common> msgLdrprepares, std::set<ReplicaID> signers)
{
	for (std::set<MsgLdrprepareFast2Common>::iterator itMsg = msgLdrprepares.begin(); itMsg != msgLdrprepares.end(); itMsg++)
	{
		MsgLdrprepareFast2Common msgLdrprepare = *itMsg;
		std::set<ReplicaID> allSigners = msgLdrprepare.signs.getSigners();
		for (std::set<ReplicaID>::iterator itSigner = allSigners.begin(); itSigner != allSigners.end(); itSigner++)
		{
			signers.erase(*itSigner);
			if (signers.empty())
			{
				return true;
			}
		}
	}
	return false;
}

unsigned int Log::storeMsgLdrprepareFast2Common(MsgLdrprepareFast2Common msgLdrprepare)
{
	ProposalCommon proposal_MsgLdrprepare = msgLdrprepare.proposalCommon;
	View proposeView_MsgLdrprepare = proposal_MsgLdrprepare.getJustification().getRoundData().getProposeView();
	std::set<ReplicaID> signers = msgLdrprepare.signs.getSigners();
	std::map<View, std::set<MsgLdrprepareFast2Common>>::iterator itView = this->ldrpreparesFast2Common.find(proposeView_MsgLdrprepare);
	if (itView != this->ldrpreparesFast2Common.end())
	{
		std::set<MsgLdrprepareFast2Common> msgLdrprepares = itView->second;
		if (!MsgLdrprepareFast2CommonFrom(msgLdrprepares, signers))
		{
			msgLdrprepares.insert(msgLdrprepare);
			this->ldrpreparesFast2Common[proposeView_MsgLdrprepare] = msgLdrprepares;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Updated entry for msgLdrprepare for fast path to common path in view " << proposeView_MsgLdrprepare << " and the number of msgLdrprepare is: " << msgLdrprepares.size() << COLOUR_NORMAL << std::endl;
			}
			return msgLdrprepares.size();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		std::set<MsgLdrprepareFast2Common> msgLdrprepares = {msgLdrprepare};
		this->ldrpreparesFast2Common[proposeView_MsgLdrprepare] = msgLdrprepares;
		if (DEBUG_LOG)
		{
			std::cout << COLOUR_GREEN << "No entry for msgLdrprepare for fast path to common path in view " << proposeView_MsgLdrprepare << " and the number of msgLdrprepare is: 1" << COLOUR_NORMAL << std::endl;
		}
		return 1;
	}
}

bool MsgPrepareFast2CommonFrom(std::set<MsgPrepareFast2Common> msgPrepares, std::set<ReplicaID> signers)
{
	for (std::set<MsgPrepareFast2Common>::iterator itMsg = msgPrepares.begin(); itMsg != msgPrepares.end(); itMsg++)
	{
		MsgPrepareFast2Common msgPrepare = *itMsg;
		std::set<ReplicaID> allSigners = msgPrepare.signs.getSigners();
		for (std::set<ReplicaID>::iterator itSigner = allSigners.begin(); itSigner != allSigners.end(); itSigner++)
		{
			signers.erase(*itSigner);
			if (signers.empty())
			{
				return true;
			}
		}
	}
	return false;
}

unsigned int Log::storeMsgPrepareFast2Common(MsgPrepareFast2Common msgPrepare)
{
	RoundData roundData_MsgPrepare = msgPrepare.roundData;
	View proposeView_MsgPrepare = roundData_MsgPrepare.getProposeView();
	std::set<ReplicaID> signers = msgPrepare.signs.getSigners();
	std::map<View, std::set<MsgPrepareFast2Common>>::iterator itView = this->preparesFast2Common.find(proposeView_MsgPrepare);
	if (itView != this->preparesFast2Common.end())
	{
		std::set<MsgPrepareFast2Common> msgPrepares = itView->second;
		if (!MsgPrepareFast2CommonFrom(msgPrepares, signers))
		{
			msgPrepares.insert(msgPrepare);
			this->preparesFast2Common[proposeView_MsgPrepare] = msgPrepares;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Updated entry for msgPrepare for fast path to common path in view " << proposeView_MsgPrepare << " and the number of msgPrepare is: " << msgPrepares.size() << COLOUR_NORMAL << std::endl;
			}
			return msgPrepares.size();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		std::set<MsgPrepareFast2Common> msgPrepares = {msgPrepare};
		this->preparesFast2Common[proposeView_MsgPrepare] = msgPrepares;
		if (DEBUG_LOG)
		{
			std::cout << COLOUR_GREEN << "No entry for msgPrepare for fast path to common path in view " << proposeView_MsgPrepare << " and the number of msgPrepare is: 1" << COLOUR_NORMAL << std::endl;
		}
		return 1;
	}
}

bool MsgPrecommitFast2CommonFrom(std::set<MsgPrecommitFast2Common> msgPrecommits, std::set<ReplicaID> signers)
{
	for (std::set<MsgPrecommitFast2Common>::iterator itMsg = msgPrecommits.begin(); itMsg != msgPrecommits.end(); itMsg++)
	{
		MsgPrecommitFast2Common msgPrecommit = *itMsg;
		std::set<ReplicaID> allSigners = msgPrecommit.signs.getSigners();
		for (std::set<ReplicaID>::iterator itSigner = allSigners.begin(); itSigner != allSigners.end(); itSigner++)
		{
			signers.erase(*itSigner);
			if (signers.empty())
			{
				return true;
			}
		}
	}
	return false;
}

unsigned int Log::storeMsgPrecommitFast2Common(MsgPrecommitFast2Common msgPrecommit)
{
	RoundData roundData_MsgPrecommit = msgPrecommit.roundData;
	View proposeView_MsgPrecommit = roundData_MsgPrecommit.getProposeView();
	std::set<ReplicaID> signers = msgPrecommit.signs.getSigners();
	std::map<View, std::set<MsgPrecommitFast2Common>>::iterator itView = this->precommitsFast2Common.find(proposeView_MsgPrecommit);
	if (itView != this->precommitsFast2Common.end())
	{
		std::set<MsgPrecommitFast2Common> msgPrecommits = itView->second;
		if (!MsgPrecommitFast2CommonFrom(msgPrecommits, signers))
		{
			msgPrecommits.insert(msgPrecommit);
			this->precommitsFast2Common[proposeView_MsgPrecommit] = msgPrecommits;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Updated entry for msgPrecommit for fast path to common path in view " << proposeView_MsgPrecommit << " and the number of msgPrecommit is: " << msgPrecommits.size() << COLOUR_NORMAL << std::endl;
			}
			return msgPrecommits.size();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		std::set<MsgPrecommitFast2Common> msgPrecommits = {msgPrecommit};
		this->precommitsFast2Common[proposeView_MsgPrecommit] = msgPrecommits;
		if (DEBUG_LOG)
		{
			std::cout << COLOUR_GREEN << "No entry for msgPrecommit for fast path to common path in view " << proposeView_MsgPrecommit << " and the number of msgPrecommit is: 1" << COLOUR_NORMAL << std::endl;
		}
		return 1;
	}
}

bool MsgCommitFast2CommonFrom(std::set<MsgCommitFast2Common> msgCommits, std::set<ReplicaID> signers)
{
	for (std::set<MsgCommitFast2Common>::iterator itMsg = msgCommits.begin(); itMsg != msgCommits.end(); itMsg++)
	{
		MsgCommitFast2Common msgCommit = *itMsg;
		std::set<ReplicaID> allSigners = msgCommit.signs.getSigners();
		for (std::set<ReplicaID>::iterator itSigner = allSigners.begin(); itSigner != allSigners.end(); itSigner++)
		{
			signers.erase(*itSigner);
			if (signers.empty())
			{
				return true;
			}
		}
	}
	return false;
}

unsigned int Log::storeMsgCommitFast2Common(MsgCommitFast2Common msgCommit)
{
	RoundData roundData_MsgCommit = msgCommit.roundData;
	View proposeView_MsgCommit = roundData_MsgCommit.getProposeView();
	std::set<ReplicaID> signers = msgCommit.signs.getSigners();
	std::map<View, std::set<MsgCommitFast2Common>>::iterator itView = this->commitsFast2Common.find(proposeView_MsgCommit);
	if (itView != this->commitsFast2Common.end())
	{
		std::set<MsgCommitFast2Common> msgCommits = itView->second;
		if (!MsgCommitFast2CommonFrom(msgCommits, signers))
		{
			msgCommits.insert(msgCommit);
			this->commitsFast2Common[proposeView_MsgCommit] = msgCommits;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Updated entry for msgCommit for fast path to common path in view " << proposeView_MsgCommit << " and the number of msgCommit is: " << msgCommits.size() << COLOUR_NORMAL << std::endl;
			}
			return msgCommits.size();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		std::set<MsgCommitFast2Common> msgCommits = {msgCommit};
		this->commitsFast2Common[proposeView_MsgCommit] = msgCommits;
		if (DEBUG_LOG)
		{
			std::cout << COLOUR_GREEN << "No entry for msgCommit for fast path to common path in view " << proposeView_MsgCommit << " and the number of msgCommit is: 1" << COLOUR_NORMAL << std::endl;
		}
		return 1;
	}
}

Signs Log::getMsgPrepareFast2Common(View view, unsigned int n)
{
	Signs signs;
	std::map<View, std::set<MsgPrepareFast2Common>>::iterator itView = this->preparesFast2Common.find(view);
	if (itView != this->preparesFast2Common.end())
	{
		std::set<MsgPrepareFast2Common> msgPrepares = itView->second;
		for (std::set<MsgPrepareFast2Common>::iterator itMsg = msgPrepares.begin(); signs.getSize() < n && itMsg != msgPrepares.end(); itMsg++)
		{
			MsgPrepareFast2Common msgPrepare = *itMsg;
			Signs signs_MsgPrepare = msgPrepare.signs;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Adding signatures: " << signs_MsgPrepare.toPrint() << COLOUR_NORMAL << std::endl;
			}
			signs.addUpto(signs_MsgPrepare, n);
		}
	}
	if (DEBUG_LOG)
	{
		std::cout << COLOUR_GREEN << "Log signatures: " << signs.toPrint() << COLOUR_NORMAL << std::endl;
	}
	return signs;
}

Signs Log::getMsgPrecommitFast2Common(View view, unsigned int n)
{
	Signs signs;
	std::map<View, std::set<MsgPrecommitFast2Common>>::iterator itView = this->precommitsFast2Common.find(view);
	if (itView != this->precommitsFast2Common.end())
	{
		std::set<MsgPrecommitFast2Common> msgPrecommits = itView->second;
		for (std::set<MsgPrecommitFast2Common>::iterator itMsg = msgPrecommits.begin(); signs.getSize() < n && itMsg != msgPrecommits.end(); itMsg++)
		{
			MsgPrecommitFast2Common msgPrecommit = *itMsg;
			Signs signs_MsgPrecommit = msgPrecommit.signs;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Adding signatures: " << signs_MsgPrecommit.toPrint() << COLOUR_NORMAL << std::endl;
			}
			signs.addUpto(signs_MsgPrecommit, n);
		}
	}
	if (DEBUG_LOG)
	{
		std::cout << COLOUR_GREEN << "Log signatures: " << signs.toPrint() << COLOUR_NORMAL << std::endl;
	}
	return signs;
}

Signs Log::getMsgCommitFast2Common(View view, unsigned int n)
{
	Signs signs;
	std::map<View, std::set<MsgCommitFast2Common>>::iterator itView = this->commitsFast2Common.find(view);
	if (itView != this->commitsFast2Common.end())
	{
		std::set<MsgCommitFast2Common> msgCommits = itView->second;
		for (std::set<MsgCommitFast2Common>::iterator itMsg = msgCommits.begin(); signs.getSize() < n && itMsg != msgCommits.end(); itMsg++)
		{
			MsgCommitFast2Common msgCommit = *itMsg;
			Signs signs_MsgCommit = msgCommit.signs;
			if (DEBUG_LOG)
			{
				std::cout << COLOUR_GREEN << "Adding signatures: " << signs_MsgCommit.toPrint() << COLOUR_NORMAL << std::endl;
			}
			signs.addUpto(signs_MsgCommit, n);
		}
	}
	if (DEBUG_LOG)
	{
		std::cout << COLOUR_GREEN << "Log signatures: " << signs.toPrint() << COLOUR_NORMAL << std::endl;
	}
	return signs;
}

MsgLdrprepareFast2Common Log::firstMsgLdrprepareFast2Common(View view)
{
	std::map<View, std::set<MsgLdrprepareFast2Common>>::iterator itView = this->ldrpreparesFast2Common.find(view);
	if (itView != this->ldrpreparesFast2Common.end())
	{
		std::set<MsgLdrprepareFast2Common> msgLdrprepares = itView->second;
		if (msgLdrprepares.size() > 0)
		{
			std::set<MsgLdrprepareFast2Common>::iterator itMsg = msgLdrprepares.begin();
			MsgLdrprepareFast2Common msgLdrprepare = *itMsg;
			return msgLdrprepare;
		}
	}
	ProposalCommon proposal;
	Committee committee;
	Signs signs;
	MsgLdrprepareFast2Common msgLdrprepare = MsgLdrprepareFast2Common(proposal, committee, signs);
	return msgLdrprepare;
}

MsgPrepareFast2Common Log::firstMsgPrepareFast2Common(View view)
{
	std::map<View, std::set<MsgPrepareFast2Common>>::iterator itView = this->preparesFast2Common.find(view);
	if (itView != this->preparesFast2Common.end())
	{
		std::set<MsgPrepareFast2Common> msgPrepares = itView->second;
		if (msgPrepares.size() > 0)
		{
			std::set<MsgPrepareFast2Common>::iterator itMsg = msgPrepares.begin();
			MsgPrepareFast2Common msgPrepare = *itMsg;
			return msgPrepare;
		}
	}
	RoundData roundData;
	Signs signs;
	MsgPrepareFast2Common msgPrepare = MsgPrepareFast2Common(roundData, signs);
	return msgPrepare;
}

MsgPrecommitFast2Common Log::firstMsgPrecommitFast2Common(View view)
{
	std::map<View, std::set<MsgPrecommitFast2Common>>::iterator itView = this->precommitsFast2Common.find(view);
	if (itView != this->precommitsFast2Common.end())
	{
		std::set<MsgPrecommitFast2Common> msgPrecommits = itView->second;
		if (msgPrecommits.size() > 0)
		{
			std::set<MsgPrecommitFast2Common>::iterator itMsg = msgPrecommits.begin();
			MsgPrecommitFast2Common msgPrecommit = *itMsg;
			return msgPrecommit;
		}
	}
	RoundData roundData;
	Signs signs;
	MsgPrecommitFast2Common msgPrecommit = MsgPrecommitFast2Common(roundData, signs);
	return msgPrecommit;
}

MsgCommitFast2Common Log::firstMsgCommitFast2Common(View view)
{
	std::map<View, std::set<MsgCommitFast2Common>>::iterator itView = this->commitsFast2Common.find(view);
	if (itView != this->commitsFast2Common.end())
	{
		std::set<MsgCommitFast2Common> msgCommits = itView->second;
		if (msgCommits.size() > 0)
		{
			std::set<MsgCommitFast2Common>::iterator itMsg = msgCommits.begin();
			MsgCommitFast2Common msgCommit = *itMsg;
			return msgCommit;
		}
	}
	RoundData roundData;
	Signs signs;
	MsgCommitFast2Common msgCommit = MsgCommitFast2Common(roundData, signs);
	return msgCommit;
}

std::string Log::toPrint()
{
	std::string text = "";

	// MsgNewviewCommon
	for (std::map<View, std::set<MsgNewviewCommon>>::iterator itView = this->newviewsCommon.begin(); itView != this->newviewsCommon.end(); itView++)
	{
		View view = itView->first;
		std::set<MsgNewviewCommon> msgs = itView->second;
		text += "MsgNewviewCommon: View = " + std::to_string(view) + "; The number of MsgNewview in common path is: " + std::to_string(msgs.size()) + "\n";
	}
	// MsgLdrprepareCommon
	for (std::map<View, std::set<MsgLdrprepareCommon>>::iterator itView = this->ldrpreparesCommon.begin(); itView != this->ldrpreparesCommon.end(); itView++)
	{
		View view = itView->first;
		std::set<MsgLdrprepareCommon> msgs = itView->second;
		text += "MsgLdrprepareCommon: View = " + std::to_string(view) + "; The number of MsgLdrprepare in common path is: " + std::to_string(msgs.size()) + "\n";
	}
	// MsgPrepareCommon
	for (std::map<View, std::set<MsgPrepareCommon>>::iterator itView = this->preparesCommon.begin(); itView != this->preparesCommon.end(); itView++)
	{
		View view = itView->first;
		std::set<MsgPrepareCommon> msgs = itView->second;
		text += "MsgPrepareCommon: View = " + std::to_string(view) + "; The number of MsgPrepare in common path is: " + std::to_string(msgs.size()) + "\n";
	}
	// MsgPrecommitCommon
	for (std::map<View, std::set<MsgPrecommitCommon>>::iterator itView = this->precommitsCommon.begin(); itView != this->precommitsCommon.end(); itView++)
	{
		View view = itView->first;
		std::set<MsgPrecommitCommon> msgs = itView->second;
		text += "MsgPrecommitCommon: View = " + std::to_string(view) + "; The number of MsgPrecommit in common path is: " + std::to_string(msgs.size()) + "\n";
	}
	// MsgCommitCommon
	for (std::map<View, std::set<MsgCommitCommon>>::iterator itView = this->commitsCommon.begin(); itView != this->commitsCommon.end(); itView++)
	{
		View view = itView->first;
		std::set<MsgCommitCommon> msgs = itView->second;
		text += "MsgCommitCommon: View = " + std::to_string(view) + "; The number of MsgCommit in common path is: " + std::to_string(msgs.size()) + "\n";
	}
	// MsgNewviewFast
	for (std::map<View, std::set<MsgNewviewFast>>::iterator itView = this->newviewsFast.begin(); itView != this->newviewsFast.end(); itView++)
	{
		View view = itView->first;
		std::set<MsgNewviewFast> msgs = itView->second;
		text += "MsgNewviewFast: View = " + std::to_string(view) + "; The number of MsgNewview in fast path is: " + std::to_string(msgs.size()) + "\n";
	}
	// MsgLdrprepareFast
	for (std::map<View, std::set<MsgLdrprepareFast>>::iterator itView = this->ldrpreparesFast.begin(); itView != this->ldrpreparesFast.end(); itView++)
	{
		View view = itView->first;
		std::set<MsgLdrprepareFast> msgs = itView->second;
		text += "MsgLdrprepareFast: View = " + std::to_string(view) + "; The number of MsgLdrprepare in fast path is: " + std::to_string(msgs.size()) + "\n";
	}
	// MsgPrepareFast
	for (std::map<View, std::set<MsgPrepareFast>>::iterator itView = this->preparesFast.begin(); itView != this->preparesFast.end(); itView++)
	{
		View view = itView->first;
		std::set<MsgPrepareFast> msgs = itView->second;
		text += "MsgPrepareFast: View = " + std::to_string(view) + "; The number of MsgPrepareFast in fast path is: " + std::to_string(msgs.size()) + "\n";
	}
	// MsgPrecommitFast
	for (std::map<View, std::set<MsgPrecommitFast>>::iterator itView = this->precommitsFast.begin(); itView != this->precommitsFast.end(); itView++)
	{
		View view = itView->first;
		std::set<MsgPrecommitFast> msgs = itView->second;
		text += "MsgPrecommitFast: View = " + std::to_string(view) + "; The number of MsgPrecommitFast in fast path is: " + std::to_string(msgs.size()) + "\n";
	}
	// MsgLdrprepareFast2Common
	for (std::map<View, std::set<MsgLdrprepareFast2Common>>::iterator itView = this->ldrpreparesFast2Common.begin(); itView != this->ldrpreparesFast2Common.end(); itView++)
	{
		View view = itView->first;
		std::set<MsgLdrprepareFast2Common> msgs = itView->second;
		text += "MsgLdrprepareFast2Common: View = " + std::to_string(view) + "; The number of MsgLdrprepare for fast path to common path is: " + std::to_string(msgs.size()) + "\n";
	}
	// MsgPrepareFast2Common
	for (std::map<View, std::set<MsgPrepareFast2Common>>::iterator itView = this->preparesFast2Common.begin(); itView != this->preparesFast2Common.end(); itView++)
	{
		View view = itView->first;
		std::set<MsgPrepareFast2Common> msgs = itView->second;
		text += "MsgPrepareFast2Common: View = " + std::to_string(view) + "; The number of MsgPrepare for fast path to common path is: " + std::to_string(msgs.size()) + "\n";
	}
	// MsgPrecommitFast2Common
	for (std::map<View, std::set<MsgPrecommitFast2Common>>::iterator itView = this->precommitsFast2Common.begin(); itView != this->precommitsFast2Common.end(); itView++)
	{
		View view = itView->first;
		std::set<MsgPrecommitFast2Common> msgs = itView->second;
		text += "MsgPrecommitFast2Common: View = " + std::to_string(view) + "; The number of MsgPrecommit for fast path to common path is: " + std::to_string(msgs.size()) + "\n";
	}
	// MsgCommitFast2Common
	for (std::map<View, std::set<MsgCommitFast2Common>>::iterator itView = this->commitsFast2Common.begin(); itView != this->commitsFast2Common.end(); itView++)
	{
		View view = itView->first;
		std::set<MsgCommitFast2Common> msgs = itView->second;
		text += "MsgCommitFast2Common: View = " + std::to_string(view) + "; The number of MsgCommit for fast path to common path is: " + std::to_string(msgs.size()) + "\n";
	}

	return text;
}