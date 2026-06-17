#include "ResiBFT.h"

// Local variables
Time startTime = std::chrono::steady_clock::now();
Time startView = std::chrono::steady_clock::now();
Time currentTime;
std::string statisticsValues;
std::string statisticsDone;
Statistics statistics;
GeneralRep generalRep;
sgx_enclave_id_t global_eid = 0;

// Opcodes
const uint8_t MsgTransaction::opcode;
const uint8_t MsgStart::opcode;
const uint8_t MsgReply::opcode;

const uint8_t MsgNewviewCommon::opcode;
const uint8_t MsgLdrprepareCommon::opcode;
const uint8_t MsgPrepareCommon::opcode;
const uint8_t MsgPrecommitCommon::opcode;
const uint8_t MsgCommitCommon::opcode;

const uint8_t MsgNewviewFast::opcode;
const uint8_t MsgLdrprepareFast::opcode;
const uint8_t MsgPrepareFast::opcode;
const uint8_t MsgPrecommitFast::opcode;
const uint8_t MsgValidationFast::opcode;

// Converts between classes and simpler structures used in enclaves
// Store [transaction] in [transaction_t]
void setTransaction(Transaction transaction, Transaction_t *transaction_t)
{
	transaction_t->clientId = transaction.getClientId();
	transaction_t->transactionId = transaction.getTransactionId();
	memcpy(transaction_t->transactionData, transaction.getTransactionData(), NUM_PAYLOAD_SIZE);
}

// Store [hash] in [hash_t]
void setHash(Hash hash, Hash_t *hash_t)
{
	memcpy(hash_t->hash, hash.getHash(), SHA256_DIGEST_LENGTH);
	hash_t->set = hash.getSet();
}

// Load [hash] from [hash_t]
Hash getHash(Hash_t *hash_t)
{
	bool set = hash_t->set;
	unsigned char *hash = hash_t->hash;
	Hash hash_ = Hash(set, hash);
	return hash_;
}

// Store [block] in [block_t]
void setBlock(Block block, Block_t *block_t)
{
	block_t->set = block.getSet();
	setHash(block.getPreviousHash(), &(block_t->previousHash));
	block_t->size = block.getSize();
	for (int i = 0; i < block.getSize(); i++)
	{
		setTransaction(block.get(i), &(block_t->transactions[i]));
	}
}

// Store [roundData] in [roundData_t]
void setRoundData(RoundData roundData, RoundData_t *roundData_t)
{
	setHash(roundData.getProposeHash(), &(roundData_t->proposeHash));
	roundData_t->proposeView = roundData.getProposeView();
	setHash(roundData.getJustifyHash(), &(roundData_t->justifyHash));
	roundData_t->justifyView = roundData.getJustifyView();
	roundData_t->phase = roundData.getPhase();
}

// Load [roundData] from [roundData_t]
RoundData getRoundData(RoundData_t *roundData_t)
{
	Hash proposeHash = getHash(&(roundData_t->proposeHash));
	View proposeView = roundData_t->proposeView;
	Hash justifyHash = getHash(&(roundData_t->justifyHash));
	View justifyView = roundData_t->justifyView;
	Phase phase = roundData_t->phase;
	RoundData roundData = RoundData(proposeHash, proposeView, justifyHash, justifyView, phase);
	return roundData;
}

// Store [sign] in [sign_t]
void setSign(Sign sign, Sign_t *sign_t)
{
	sign_t->set = sign.isSet();
	sign_t->signer = sign.getSigner();
	memcpy(sign_t->signtext, sign.getSigntext(), SIGN_LEN);
}

// Load [sign] from [sign_t]
Sign getSign(Sign_t *sign_t)
{
	bool b = sign_t->set;
	ReplicaID signer = sign_t->signer;
	unsigned char *signtext = sign_t->signtext;
	Sign sign = Sign(b, signer, signtext);
	return sign;
}

// Store [signs] in [signs_t]
void setSigns(Signs signs, Signs_t *signs_t)
{
	signs_t->size = signs.getSize();
	for (int i = 0; i < signs.getSize(); i++)
	{
		setSign(signs.get(i), &(signs_t->signs[i]));
	}
}

// Load [signs] from [signs_t]
Signs getSigns(Signs_t *signs_t)
{
	unsigned int size = signs_t->size;
	Sign signs[NUM_ACTIVE_REPLICAS];
	for (int i = 0; i < size; i++)
	{
		signs[i] = getSign(&(signs_t->signs[i]));
	}
	Signs signs_ = Signs(size, signs);
	return signs_;
}

// Store [justification] in [justification_t]
void setJustification(Justification justification, Justification_t *justification_t)
{
	justification_t->set = justification.isSet();
	setRoundData(justification.getRoundData(), &(justification_t->roundData));
	setSigns(justification.getSigns(), &(justification_t->signs));
}

// Store [justifications] in [justifications_t]
void setJustifications(Justification justifications[NUM_ACTIVE_REPLICAS], Justifications_t *justifications_t)
{
	for (int i = 0; i < NUM_ACTIVE_REPLICAS; i++)
	{
		setJustification(justifications[i], &(justifications_t->justifications[i]));
	}
}

// Load [justification] from [justification_t]
Justification getJustification(Justification_t *justification_t)
{
	bool set = justification_t->set;
	RoundData roundData = getRoundData(&(justification_t->roundData));
	Sign sign[NUM_ACTIVE_REPLICAS];
	for (int i = 0; i < NUM_ACTIVE_REPLICAS; i++)
	{
		sign[i] = Sign(justification_t->signs.signs[i].set, justification_t->signs.signs[i].signer, justification_t->signs.signs[i].signtext);
	}
	Signs signs(justification_t->signs.size, sign);
	Justification justification = Justification(set, roundData, signs);
	return justification;
}

// Store [accumulator] in [accumulator_t]
void setAccumulator(Accumulator accumulator, Accumulator_t *accumulator_t)
{
	accumulator_t->set = accumulator.isSet();
	accumulator_t->proposeView = accumulator.getProposeView();
	accumulator_t->prepareHash.set = accumulator.getPrepareHash().getSet();
	memcpy(accumulator_t->prepareHash.hash, accumulator.getPrepareHash().getHash(), SHA256_DIGEST_LENGTH);
	accumulator_t->prepareView = accumulator.getPrepareView();
	accumulator_t->size = accumulator.getSize();
}

// Load [accumulator] from [accumulator_t]
Accumulator getAccumulator(Accumulator_t *accumulator_t)
{
	bool set = accumulator_t->set;
	View proposeView = accumulator_t->proposeView;
	Hash prepareHash = getHash(&(accumulator_t->prepareHash));
	View prepareView = accumulator_t->prepareView;
	unsigned int size = accumulator_t->size;
	Accumulator accumulator = Accumulator(set, proposeView, prepareHash, prepareView, size);
	return accumulator;
}

// Store [proposalCommon] in [proposalCommon_t]
void setProposalCommon(ProposalCommon proposalCommon, ProposalCommon_t *proposalCommon_t)
{
	setJustification(proposalCommon.getJustification(), &(proposalCommon_t->justification));
	setBlock(proposalCommon.getBlock(), &(proposalCommon_t->block));
}

// Store [proposalFast] in [proposalFast_t]
void setProposalFast(ProposalFast proposalFast, ProposalFast_t *proposalFast_t)
{
	setAccumulator(proposalFast.getAccumulator(), &(proposalFast_t->accumulator));
	setBlock(proposalFast.getBlock(), &(proposalFast_t->block));
}

// Store [validation] in [validation_t]
void setValidation(Validation validation, Validation_t *validation_t)
{
	validation_t->set = validation.isSet();
	validation_t->verifier = validation.isAccepted();
}

// Load [validation] from [validation_t]
Validation getValidation(Validation_t *validation_t)
{
	bool set = validation_t->set;
	bool verifier = validation_t->verifier;
	Validation validation = Validation(set, verifier);
	return validation;
}

// Store [validations] in [validations_t]
void setValidations(Validations validations, Validations_t *validations_t)
{
	validations_t->size = validations.getSize();
	for (int i = 0; i < validations.getSize(); i++)
	{
		setValidation(validations.get(i), &(validations_t->validations[i]));
	}
}

void TEE_Print(const char *text)
{
	printf("%s\n", text);
}

// Print functions
std::string ResiBFT::printReplicaId()
{
	return "[" + std::to_string(this->replicaId) + "-" + std::to_string(this->view) + "]";
}

void ResiBFT::printNowTime(std::string msg)
{
	auto now = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(now - startView).count();
	double etime = (statistics.getTotalViewTimes() + time) / (1000 * 1000);
	std::cout << COLOUR_BLUE << this->printReplicaId() << msg << " @ " << etime << COLOUR_NORMAL << std::endl;
}

void ResiBFT::printClientInfo()
{
	for (Clients::iterator it = this->clients.begin(); it != this->clients.end(); it++)
	{
		ClientID clientId = it->first;
		ClientInformation clientInfo = it->second;
		bool running = std::get<0>(clientInfo);
		unsigned int received = std::get<1>(clientInfo);
		unsigned int replied = std::get<2>(clientInfo);
		ClientNet::conn_t conn = std::get<3>(clientInfo);
		if (DEBUG_BASIC)
		{
			std::cout << COLOUR_RED
					  << this->printReplicaId() << "CLIENT[id: "
					  << clientId << ", running: "
					  << running << ", numbers of received: "
					  << received << ", numbers of replied: "
					  << replied << "]" << COLOUR_NORMAL << std::endl;
		}
	}
}

std::string ResiBFT::recipients2string(Peers recipients)
{
	std::string text = "";
	for (Peers::iterator it = recipients.begin(); it != recipients.end(); it++)
	{
		Peer peer = *it;
		text += std::to_string(std::get<0>(peer)) + " ";
	}
	return text;
}

// Setting functions
bool ResiBFT::amGeneralReplicaIds()
{
	if (this->generalRecords.find(this->replicaId) != this->generalRecords.end())
	{
		return true;
	}
	return false;
}

bool ResiBFT::isGeneralReplicaIds(ReplicaID replicaId)
{
	if (this->generalRecords.find(replicaId) != this->generalRecords.end())
	{
		return true;
	}
	return false;
}

bool ResiBFT::amTrustedReplicaIds()
{
	if (this->trustedRecords.find(this->replicaId) != this->trustedRecords.end())
	{
		return true;
	}
	return false;
}

bool ResiBFT::isTrustedReplicaIds(ReplicaID replicaId)
{
	if (this->trustedRecords.find(replicaId) != this->trustedRecords.end())
	{
		return true;
	}
	return false;
}

bool ResiBFT::amTrustfailReplicaIds()
{
	if (this->trustfailRecords.find(this->replicaId) != this->trustfailRecords.end())
	{
		return true;
	}
	return false;
}

bool ResiBFT::amCommitteeReplicaIds()
{
	if (this->committee.find(this->replicaId))
	{
		return true;
	}
	return false;
}

unsigned int ResiBFT::getLeaderOf(View view)
{
	unsigned int leader;
	if (this->path == COMMON_PATH)
	{
		leader = view % this->numReplicas;
	}
	else if (this->path == FAST_PATH)
	{
		leader = this->committee.getCommittee()[view % NUM_COMMITTEE_MEMBERS];
	}
	return leader;
}

unsigned int ResiBFT::getCurrentLeader()
{
	unsigned int leader = this->getLeaderOf(this->view);
	return leader;
}

bool ResiBFT::amLeaderOf(View view)
{
	if (this->replicaId == this->getLeaderOf(view))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ResiBFT::amCurrentLeader()
{
	if (this->replicaId == this->getCurrentLeader())
	{
		return true;
	}
	else
	{
		return false;
	}
}

Peers ResiBFT::removeFromPeers(ReplicaID replicaId)
{
	Peers peers;
	for (Peers::iterator itPeers = this->peers.begin(); itPeers != this->peers.end(); itPeers++)
	{
		Peer peer = *itPeers;
		ReplicaID peerId = std::get<0>(peer);
		if (peerId != replicaId)
		{
			peers.push_back(peer);
		}
	}
	return peers;
}

Peers ResiBFT::removeFromCommitteePeers(Committee committee, ReplicaID replicaId)
{
	Peers peers;
	for (Peers::iterator itPeers = this->peers.begin(); itPeers != this->peers.end(); itPeers++)
	{
		Peer peer = *itPeers;
		ReplicaID peerId = std::get<0>(peer);
		if (peerId != replicaId && committee.find(peerId))
		{
			peers.push_back(peer);
		}
	}
	return peers;
}

Peers ResiBFT::keepFromPeers(ReplicaID replicaId)
{
	Peers peers;
	for (Peers::iterator itPeers = this->peers.begin(); itPeers != this->peers.end(); itPeers++)
	{
		Peer peer = *itPeers;
		ReplicaID peerId = std::get<0>(peer);
		if (peerId == replicaId)
		{
			peers.push_back(peer);
		}
	}
	return peers;
}

Peers ResiBFT::keepFromCommitteePeers(Committee committee, ReplicaID replicaId)
{
	Peers peers;
	for (Peers::iterator itPeers = this->peers.begin(); itPeers != this->peers.end(); itPeers++)
	{
		Peer peer = *itPeers;
		ReplicaID peerId = std::get<0>(peer);
		if (peerId != replicaId && !(committee.find(peerId)))
		{
			peers.push_back(peer);
		}
	}
	return peers;
}

std::vector<salticidae::PeerId> ResiBFT::getPeerIds(Peers recipients)
{
	std::vector<salticidae::PeerId> returnPeerId;
	for (Peers::iterator it = recipients.begin(); it != recipients.end(); it++)
	{
		Peer peer = *it;
		returnPeerId.push_back(std::get<1>(peer));
	}
	return returnPeerId;
}

void ResiBFT::setTimer()
{
	this->timer.del();
	this->timer.add(this->leaderChangeTime);
	this->timerView = this->view;
}

Committee ResiBFT::selectRandomCommittee(std::set<ReplicaID> trustedRecords)
{
	Committee committee = Committee();
	if (trustedRecords.size() < NUM_COMMITTEE_MEMBERS)
	{
		return committee;
	}

	std::set<ReplicaID> members;
	std::vector<ReplicaID> candidates(trustedRecords.begin(), trustedRecords.end());

	// Use [view] as the deterministic seed
	std::mt19937 gen(static_cast<unsigned int>(this->view));
	std::shuffle(candidates.begin(), candidates.end(), gen);

	for (unsigned int i = 0; i < NUM_COMMITTEE_MEMBERS; i++)
	{
		members.insert(candidates[i]);
	}

	committee = Committee(members);
	return committee;
}

Block ResiBFT::createNewBlock(Hash hash)
{
	std::lock_guard<std::mutex> guard(mutexTransaction);
	Transaction transaction[NUM_TRANSACTIONS];
	unsigned int i = 0;

	// We fill the block we have with transactions we have received so far
	while (i < NUM_TRANSACTIONS && !this->transactions.empty())
	{
		transaction[i] = this->transactions.front();
		this->transactions.pop_front();
		i++;
	}

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Filled block with " << i << " transactions" << COLOUR_NORMAL << std::endl;
	}

	unsigned int size = i;
	// We fill the rest with dummy transactions
	while (i < NUM_TRANSACTIONS)
	{
		transaction[i] = Transaction();
		i++;
	}
	Block block = Block(hash, size, transaction);
	return block;
}

void ResiBFT::executeBlockCommon(RoundData roundData_MsgCommitCommon)
{
	auto endView = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(endView - startView).count();
	startView = endView;
	statistics.increaseExecuteViews();
	statistics.addAllViewTimes(this->view, time);
	statistics.addAllHandleTimes(this->view);

	if (this->transactions.empty())
	{
		this->viewsWithoutNewTrans++;
	}
	else
	{
		this->viewsWithoutNewTrans = 0;
	}

	// Execute
	if (DEBUG_BASIC)
	{
		std::cout << COLOUR_RED << this->printReplicaId()
				  << "RESIBFT-EXECUTE-COMMON(" << this->view << "/" << std::to_string(this->numViews - 1) << ":" << time << ") "
				  << statistics.toString() << COLOUR_NORMAL << std::endl;
	}

	this->replyHash(roundData_MsgCommitCommon.getProposeHash());

	if (this->firstFast)
	{
		this->firstFast = false;
	}

	// Check if switch the path
	if (this->committee.isSet() && !this->committee.isEmpty())
	{
		this->path = FAST_PATH;
		if (DEBUG_BASIC)
		{
			std::cout << COLOUR_RED << this->printReplicaId() << "Changed to fast path observed by the variable" << COLOUR_NORMAL << std::endl;
		}
		statistics.addCommon2fast(roundData_MsgCommitCommon.getProposeView());
	}
	if (this->committee.isEmpty())
	{
		this->path = COMMON_PATH;
		if (DEBUG_BASIC)
		{
			std::cout << COLOUR_RED << this->printReplicaId() << "Changed to common path observed by the variable" << COLOUR_NORMAL << std::endl;
		}
		statistics.addFast2common(roundData_MsgCommitCommon.getProposeView());
	}

	if (this->timeToStop())
	{
		this->recordStatistics();
	}
	else
	{
		if (this->path == COMMON_PATH)
		{
			this->startNewViewCommon();
		}
		else
		{
			if (DEBUG_BASIC)
			{
				std::cout << COLOUR_RED << this->printReplicaId() << "Change from common path to fast path" << COLOUR_NORMAL << std::endl;
			}
			this->common2fast();

			Hash proposeHash_MsgCommitCommon = roundData_MsgCommitCommon.getProposeHash();
			View proposeView_MsgCommitCommon = roundData_MsgCommitCommon.getProposeView();

			this->initializeCheckpoint(proposeHash_MsgCommitCommon, proposeView_MsgCommitCommon);
			if (DEBUG_BASIC)
			{
				std::cout << COLOUR_RED << this->printReplicaId() << "Initializing checkpoint in fast path" << COLOUR_NORMAL << std::endl;
			}

			this->firstFast = true;
			Validation validation = Validation();
			this->startNewViewFast(validation);
		}
	}
}

void ResiBFT::executeBlockFast(RoundData roundData_MsgPrecommit, Validation validation)
{
	auto endView = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(endView - startView).count();
	startView = endView;
	statistics.increaseExecuteViews();
	statistics.addAllViewTimes(this->view, time);
	statistics.addAllHandleTimes(this->view);

	if (this->transactions.empty())
	{
		this->viewsWithoutNewTrans++;
	}
	else
	{
		this->viewsWithoutNewTrans = 0;
	}

	// Execute
	if (DEBUG_BASIC)
	{
		std::cout << COLOUR_RED << this->printReplicaId()
				  << "RESIBFT-EXECUTE-FAST(" << this->view << "/" << std::to_string(this->numViews - 1) << ":" << time << ") "
				  << statistics.toString() << COLOUR_NORMAL << std::endl;
	}

	this->replyHash(roundData_MsgPrecommit.getProposeHash());

	if (this->firstFast)
	{
		this->firstFast = false;
	}

	if (this->timeToStop())
	{
		this->recordStatistics();
	}
	else
	{
		this->startNewViewFast(validation);
	}
}

// Reply to clients
void ResiBFT::replyTransactions(Transaction *transactions)
{
	for (int i = 0; i < NUM_TRANSACTIONS; i++)
	{
		Transaction transaction = transactions[i];
		ClientID clientId = transaction.getClientId();
		TransactionID transactionId = transaction.getTransactionId(); // TransactionID 0 is for dummy transactions
		if (transactionId != 0)
		{
			Clients::iterator itClient = this->clients.find(clientId);
			if (itClient != this->clients.end())
			{
				this->executionQueue.enqueue(std::make_pair(transactionId, clientId));
				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Sending reply to " << clientId << ": " << transactionId << COLOUR_NORMAL << std::endl;
				}
			}
			else
			{
				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Unknown client: " << clientId << COLOUR_NORMAL << std::endl;
				}
			}
		}
	}
}

void ResiBFT::replyHash(Hash hash)
{
	std::map<View, Block>::iterator it = this->blocks.find(this->view);
	if (it != this->blocks.end())
	{
		Block block = it->second;
		if (hash == block.hash())
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Found block for view " << this->view << ": " << block.toPrint() << COLOUR_NORMAL << std::endl;
			}
			this->replyTransactions(block.getTransactions());
		}
		else
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Recorded block but incorrect hash for view " << this->view << COLOUR_NORMAL << std::endl;
			}
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Checking hash: " << hash.toString() << COLOUR_NORMAL << std::endl;
			}
		}
	}
	else
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "No block recorded for view " << this->view << COLOUR_NORMAL << std::endl;
		}
	}
}

// Call TEE functions
bool ResiBFT::verifyJustification(Justification justification)
{
	bool b = false;
	if (this->amTrustedReplicaIds())
	{
		Justification_t justification_t;
		setJustification(justification, &justification_t);
		sgx_status_t enclave_status_t;
		sgx_status_t ecall_status_t;
		ecall_status_t = TEE_verifyJustification(global_eid, &enclave_status_t, &justification_t, &b);
	}
	else
	{
		b = generalRep.verifyJustification(this->nodes, justification);
	}
	return b;
}

bool ResiBFT::verifyProposalCommon(ProposalCommon proposalCommon, Signs signs)
{
	bool b = false;
	if (this->amTrustedReplicaIds())
	{
		ProposalCommon_t proposalCommon_t;
		setProposalCommon(proposalCommon, &proposalCommon_t);
		Signs_t signs_t;
		setSigns(signs, &signs_t);
		sgx_status_t enclave_status_t;
		sgx_status_t ecall_status_t;
		ecall_status_t = TEE_verifyProposalCommon(global_eid, &enclave_status_t, &proposalCommon_t, &signs_t, &b);
	}
	else
	{
		b = generalRep.verifyProposalCommon(this->nodes, proposalCommon, signs);
	}
	return b;
}

bool ResiBFT::verifyProposalFast(ProposalFast proposalFast, Signs signs)
{
	bool b = false;
	if (this->amTrustedReplicaIds())
	{
		ProposalFast_t proposalFast_t;
		setProposalFast(proposalFast, &proposalFast_t);
		Signs_t signs_t;
		setSigns(signs, &signs_t);
		sgx_status_t enclave_status_t;
		sgx_status_t ecall_status_t;
		ecall_status_t = TEE_verifyProposalFast(global_eid, &enclave_status_t, &proposalFast_t, &signs_t, &b);
	}
	else
	{
		b = generalRep.verifyProposalFast(this->nodes, proposalFast, signs);
	}
	return b;
}

Justification ResiBFT::initializeMsgNewviewCommon()
{
	Justification justification_MsgNewviewCommon = Justification();
	if (this->amTrustedReplicaIds())
	{
		Justification_t justification_MsgNewviewCommon_t;
		sgx_status_t enclave_status_t;
		sgx_status_t ecall_status_t;
		ecall_status_t = TEE_initializeMsgNewviewCommon(global_eid, &enclave_status_t, &justification_MsgNewviewCommon_t);
		justification_MsgNewviewCommon = getJustification(&justification_MsgNewviewCommon_t);
	}
	else
	{
		justification_MsgNewviewCommon = generalRep.initializeMsgNewviewCommon();
	}
	return justification_MsgNewviewCommon;
}

Justification ResiBFT::respondProposalCommon(Hash proposeHash, Justification justification_MsgNewviewCommon)
{
	Justification justification_MsgPrepareCommon = Justification();
	if (this->amTrustedReplicaIds())
	{
		Hash_t proposeHash_t;
		setHash(proposeHash, &proposeHash_t);
		Justification_t justification_MsgNewviewCommon_t;
		setJustification(justification_MsgNewviewCommon, &justification_MsgNewviewCommon_t);
		Justification_t justification_MsgPrepareCommon_t;
		sgx_status_t enclave_status_t;
		sgx_status_t ecall_status_t;
		ecall_status_t = TEE_respondProposalCommon(global_eid, &enclave_status_t, &proposeHash_t, &justification_MsgNewviewCommon_t, &justification_MsgPrepareCommon_t);
		justification_MsgPrepareCommon = getJustification(&justification_MsgPrepareCommon_t);
	}
	else
	{
		justification_MsgPrepareCommon = generalRep.respondProposalCommon(this->nodes, proposeHash, justification_MsgNewviewCommon);
	}
	return justification_MsgPrepareCommon;
}

Signs ResiBFT::initializeMsgLdrprepareCommon(ProposalCommon proposal_MsgLdrprepareCommon)
{
	Signs signs_MsgLdrprepareCommon = Signs();
	if (this->amTrustedReplicaIds())
	{
		ProposalCommon_t proposal_MsgLdrprepareCommon_t;
		setProposalCommon(proposal_MsgLdrprepareCommon, &proposal_MsgLdrprepareCommon_t);
		Signs_t signs_MsgLdrprepareCommon_t;
		sgx_status_t enclave_status_t;
		sgx_status_t ecall_status_t;
		ecall_status_t = TEE_initializeMsgLdrprepareCommon(global_eid, &enclave_status_t, &proposal_MsgLdrprepareCommon_t, &signs_MsgLdrprepareCommon_t);
		signs_MsgLdrprepareCommon = getSigns(&signs_MsgLdrprepareCommon_t);
	}
	else
	{
		signs_MsgLdrprepareCommon = generalRep.initializeMsgLdrprepareCommon(proposal_MsgLdrprepareCommon);
	}
	return signs_MsgLdrprepareCommon;
}

Justification ResiBFT::saveMsgPrepareCommon(Justification justification_MsgPrepareCommon)
{
	Justification justification_MsgPrecommitCommon = Justification();
	if (this->amTrustedReplicaIds())
	{
		Justification_t justification_MsgPrepareCommon_t;
		setJustification(justification_MsgPrepareCommon, &justification_MsgPrepareCommon_t);
		Justification_t justification_MsgPrecommitCommon_t;
		sgx_status_t enclave_status_t;
		sgx_status_t ecall_status_t;
		ecall_status_t = TEE_saveMsgPrepareCommon(global_eid, &enclave_status_t, &justification_MsgPrepareCommon_t, &justification_MsgPrecommitCommon_t);
		justification_MsgPrecommitCommon = getJustification(&justification_MsgPrecommitCommon_t);
	}
	else
	{
		justification_MsgPrecommitCommon = generalRep.saveMsgPrepareCommon(this->nodes, justification_MsgPrepareCommon);
	}
	return justification_MsgPrecommitCommon;
}

Justification ResiBFT::lockMsgPrecommitCommon(Justification justification_MsgPrecommitCommon)
{
	Justification justification_MsgCommitCommon = Justification();
	if (this->amTrustedReplicaIds())
	{
		Justification_t justification_MsgPrecommitCommon_t;
		setJustification(justification_MsgPrecommitCommon, &justification_MsgPrecommitCommon_t);
		Justification_t justification_MsgCommitCommon_t;
		sgx_status_t enclave_status_t;
		sgx_status_t ecall_status_t;
		ecall_status_t = TEE_lockMsgPrecommitCommon(global_eid, &enclave_status_t, &justification_MsgPrecommitCommon_t, &justification_MsgCommitCommon_t);
		justification_MsgCommitCommon = getJustification(&justification_MsgCommitCommon_t);
	}
	else
	{
		justification_MsgCommitCommon = generalRep.lockMsgPrecommitCommon(this->nodes, justification_MsgPrecommitCommon);
	}
	return justification_MsgCommitCommon;
}

Justification ResiBFT::initializeMsgNewviewFast()
{
	Justification justification_MsgNewviewFast = Justification();
	if (this->amTrustedReplicaIds())
	{
		Justification_t justification_MsgNewviewFast_t;
		sgx_status_t enclave_status_t;
		sgx_status_t ecall_status_t;
		ecall_status_t = TEE_initializeMsgNewviewFast(global_eid, &enclave_status_t, &justification_MsgNewviewFast_t);
		justification_MsgNewviewFast = getJustification(&justification_MsgNewviewFast_t);
	}
	else
	{
		justification_MsgNewviewFast = generalRep.initializeMsgNewviewFast();
	}
	return justification_MsgNewviewFast;
}

Accumulator ResiBFT::initializeAccumulatorFast(Justification justifications_MsgNewviewFast[NUM_ACTIVE_REPLICAS])
{
	Accumulator accumulator_MsgLdrprepareFast = Accumulator();
	Justifications_t justifications_MsgNewviewFast_t;
	setJustifications(justifications_MsgNewviewFast, &justifications_MsgNewviewFast_t);
	Accumulator_t accumulator_MsgLdrprepareFast_t;
	sgx_status_t enclave_status_t;
	sgx_status_t ecall_status_t;
	ecall_status_t = TEE_initializeAccumulatorFast(global_eid, &enclave_status_t, &justifications_MsgNewviewFast_t, &accumulator_MsgLdrprepareFast_t);
	accumulator_MsgLdrprepareFast = getAccumulator(&accumulator_MsgLdrprepareFast_t);
	return accumulator_MsgLdrprepareFast;
}

Justification ResiBFT::respondProposalFast(Hash proposeHash, Accumulator accumulator_MsgLdrprepareFast)
{
	Justification justification_MsgPrepareFast = Justification();
	Accumulator_t accumulator_MsgLdrprepareFast_t;
	setAccumulator(accumulator_MsgLdrprepareFast, &accumulator_MsgLdrprepareFast_t);
	Hash_t proposeHash_t;
	setHash(proposeHash, &proposeHash_t);
	Justification_t justification_MsgPrepareFast_t;
	sgx_status_t enclave_status_t;
	sgx_status_t ecall_status_t;
	ecall_status_t = TEE_respondProposalFast(global_eid, &enclave_status_t, &proposeHash_t, &accumulator_MsgLdrprepareFast_t, &justification_MsgPrepareFast_t);
	justification_MsgPrepareFast = getJustification(&justification_MsgPrepareFast_t);
	return justification_MsgPrepareFast;
}

Signs ResiBFT::initializeMsgLdrprepareFast(ProposalFast proposal_MsgLdrprepareFast)
{
	Signs signs_MsgLdrprepareFast = Signs();
	ProposalFast_t proposal_MsgLdrprepareFast_t;
	setProposalFast(proposal_MsgLdrprepareFast, &proposal_MsgLdrprepareFast_t);
	Signs_t signs_MsgLdrprepareFast_t;
	sgx_status_t enclave_status_t;
	sgx_status_t ecall_status_t;
	ecall_status_t = TEE_initializeMsgLdrprepareFast(global_eid, &enclave_status_t, &proposal_MsgLdrprepareFast_t, &signs_MsgLdrprepareFast_t);
	signs_MsgLdrprepareFast = getSigns(&signs_MsgLdrprepareFast_t);
	return signs_MsgLdrprepareFast;
}

Justification ResiBFT::saveMsgPrepareFast(Justification justification_MsgPrepareFast, bool isFail_MsgPrepareFast)
{
	Justification_t justification_MsgPrepareFast_t;
	setJustification(justification_MsgPrepareFast, &justification_MsgPrepareFast_t);
	Justification_t justification_MsgPrecommit_t;
	sgx_status_t enclave_status_t;
	sgx_status_t ecall_status_t;
	ecall_status_t = TEE_saveMsgPrepareFast(global_eid, &enclave_status_t, &justification_MsgPrepareFast_t, &isFail_MsgPrepareFast, &justification_MsgPrecommit_t);
	Justification justification_MsgPrecommit = getJustification(&justification_MsgPrecommit_t);
	return justification_MsgPrecommit;
}

Justification ResiBFT::respondProposalFast2Common(Hash proposeHash, Justification justification_MsgNewviewFast)
{
	Justification justification_MsgPrepareCommon = Justification();
	if (this->amTrustedReplicaIds())
	{
		Hash_t proposeHash_t;
		setHash(proposeHash, &proposeHash_t);
		Justification_t justification_MsgNewviewFast_t;
		setJustification(justification_MsgNewviewFast, &justification_MsgNewviewFast_t);
		Justification_t justification_MsgPrepareCommon_t;
		sgx_status_t enclave_status_t;
		sgx_status_t ecall_status_t;
		ecall_status_t = TEE_respondProposalFast2Common(global_eid, &enclave_status_t, &proposeHash_t, &justification_MsgNewviewFast_t, &justification_MsgPrepareCommon_t);
		justification_MsgPrepareCommon = getJustification(&justification_MsgPrepareCommon_t);
	}
	else
	{
		justification_MsgPrepareCommon = generalRep.respondProposalFast2Common(this->nodes, proposeHash, justification_MsgNewviewFast);
	}
	return justification_MsgPrepareCommon;
}

Signs ResiBFT::initializeMsgLdrprepareFast2Common(ProposalCommon proposal_MsgLdrprepareFast2Common)
{
	Signs signs_MsgLdrprepareFast2Common = Signs();
	ProposalCommon_t proposal_MsgLdrprepareFast2Common_t;
	setProposalCommon(proposal_MsgLdrprepareFast2Common, &proposal_MsgLdrprepareFast2Common_t);
	Signs_t signs_MsgLdrprepareFast2Common_t;
	sgx_status_t enclave_status_t;
	sgx_status_t ecall_status_t;
	ecall_status_t = TEE_initializeMsgLdrprepareFast2Common(global_eid, &enclave_status_t, &proposal_MsgLdrprepareFast2Common_t, &signs_MsgLdrprepareFast2Common_t);
	signs_MsgLdrprepareFast2Common = getSigns(&signs_MsgLdrprepareFast2Common_t);
	return signs_MsgLdrprepareFast2Common;
}

Justification ResiBFT::saveMsgPrepareFast2Common(Justification justification_MsgPrepareFast2Common)
{
	Justification justification_MsgPrecommitFast2Common = Justification();
	if (this->amTrustedReplicaIds())
	{
		Justification_t justification_MsgPrepareFast2Common_t;
		setJustification(justification_MsgPrepareFast2Common, &justification_MsgPrepareFast2Common_t);
		Justification_t justification_MsgPrecommitFast2Common_t;
		sgx_status_t enclave_status_t;
		sgx_status_t ecall_status_t;
		ecall_status_t = TEE_saveMsgPrepareFast2Common(global_eid, &enclave_status_t, &justification_MsgPrepareFast2Common_t, &justification_MsgPrecommitFast2Common_t);
		justification_MsgPrecommitFast2Common = getJustification(&justification_MsgPrecommitFast2Common_t);
	}
	else
	{
		justification_MsgPrecommitFast2Common = generalRep.saveMsgPrepareFast2Common(this->nodes, justification_MsgPrepareFast2Common);
	}
	return justification_MsgPrecommitFast2Common;
}

Justification ResiBFT::lockMsgPrecommitFast2Common(Justification justification_MsgPrecommitFast2Common)
{
	Justification justification_MsgCommitFast2Common = Justification();
	if (this->amTrustedReplicaIds())
	{
		Justification_t justification_MsgPrecommitFast2Common_t;
		setJustification(justification_MsgPrecommitFast2Common, &justification_MsgPrecommitFast2Common_t);
		Justification_t justification_MsgCommitFast2Common_t;
		sgx_status_t enclave_status_t;
		sgx_status_t ecall_status_t;
		ecall_status_t = TEE_lockMsgPrecommitFast2Common(global_eid, &enclave_status_t, &justification_MsgPrecommitFast2Common_t, &justification_MsgCommitFast2Common_t);
		justification_MsgCommitFast2Common = getJustification(&justification_MsgCommitFast2Common_t);
	}
	else
	{
		justification_MsgCommitFast2Common = generalRep.lockMsgPrecommitFast2Common(this->nodes, justification_MsgPrecommitFast2Common);
	}
	return justification_MsgCommitFast2Common;
}

Validations ResiBFT::buildValidations(std::set<MsgNewviewFast> msgNewviewFasts)
{
	Validation validations_MsgNewviewFast[NUM_ACTIVE_REPLICAS];
	unsigned int i = 0;
	for (std::set<MsgNewviewFast>::iterator itMsg = msgNewviewFasts.begin(); itMsg != msgNewviewFasts.end() && i < NUM_ACTIVE_REPLICAS; itMsg++, i++)
	{
		MsgNewviewFast msgNewviewFast = *itMsg;
		bool isFail_MsgNewviewFast = msgNewviewFast.isFail;
		Validation validation_MsgNewviewFast = msgNewviewFast.validation;
		if (!isFail_MsgNewviewFast)
		{
			validations_MsgNewviewFast[i] = validation_MsgNewviewFast;
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Validation of MsgNewview[" << i << "] in fast path: " << validations_MsgNewviewFast[i].toPrint() << COLOUR_NORMAL << std::endl;
			}
		}
		else
		{
			bool set_Validation = true;
			bool verifier_Validation = false;
			Validation failValidation = Validation(set_Validation, verifier_Validation);
			validations_MsgNewviewFast[i] = failValidation;
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Validation of MsgNewview[" << i << "] in fast path: " << validations_MsgNewviewFast[i].toPrint() << COLOUR_NORMAL << std::endl;
			}
		}
	}

	Validations validations_MsgLdrprepareFast;
	validations_MsgLdrprepareFast = Validations(i, validations_MsgNewviewFast);
	return validations_MsgLdrprepareFast;
}

Accumulator ResiBFT::buildAccumulator(std::set<MsgNewviewFast> msgNewviewFasts)
{
	Justification justifications_MsgNewviewFast[NUM_ACTIVE_REPLICAS];
	unsigned int i = 0;
	for (std::set<MsgNewviewFast>::iterator itMsg = msgNewviewFasts.begin(); itMsg != msgNewviewFasts.end() && i < NUM_ACTIVE_REPLICAS; itMsg++, i++)
	{
		MsgNewviewFast msgNewviewFast = *itMsg;
		RoundData roundData_MsgNewviewFast = msgNewviewFast.roundData;
		Signs signs_MsgNewviewFast = msgNewviewFast.signs;
		Justification justification_MsgNewviewFast = Justification(roundData_MsgNewviewFast, signs_MsgNewviewFast);
		justifications_MsgNewviewFast[i] = justification_MsgNewviewFast;
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "MsgNewview[" << i << "]: " << msgNewviewFast.toPrint() << COLOUR_NORMAL << std::endl;
		}
	}

	Accumulator accumulator_MsgLdrprepareFast;
	accumulator_MsgLdrprepareFast = this->initializeAccumulatorFast(justifications_MsgNewviewFast);
	return accumulator_MsgLdrprepareFast;
}

// Change path
void ResiBFT::common2fast()
{
	if (this->amTrustedReplicaIds())
	{
		sgx_status_t enclave_status_t;
		sgx_status_t ecall_status_t;
		ecall_status_t = TEE_common2fast(global_eid, &enclave_status_t);
	}
	else
	{
		generalRep.common2fast();
	}
}

void ResiBFT::fast2common()
{
	if (this->amTrustedReplicaIds())
	{
		sgx_status_t enclave_status_t;
		sgx_status_t ecall_status_t;
		ecall_status_t = TEE_fast2common(global_eid, &enclave_status_t);
	}
	else
	{
		generalRep.fast2common();
	}
}

void ResiBFT::initializeCheckpoint(Hash proposeHash, View proposeView)
{
	if (this->amTrustedReplicaIds())
	{
		Hash_t proposeHash_t;
		setHash(proposeHash, &proposeHash_t);
		sgx_status_t enclave_status_t;
		sgx_status_t ecall_status_t;
		ecall_status_t = TEE_initializeCheckpoint(global_eid, &enclave_status_t, &proposeHash_t, &proposeView);
	}
	else
	{
		generalRep.initializeCheckpoint(proposeHash, proposeView);
	}
}

void ResiBFT::updateCheckpoint(Hash verifyHash, View verifyView, Validations validations, Signs signs)
{
	if (this->amTrustedReplicaIds())
	{
		Hash_t verifyHash_t;
		setHash(verifyHash, &verifyHash_t);
		Validations_t validations_t;
		setValidations(validations, &validations_t);
		Signs_t signs_t;
		setSigns(signs, &signs_t);
		sgx_status_t enclave_status_t;
		sgx_status_t ecall_status_t;
		ecall_status_t = TEE_updateCheckpoint(global_eid, &enclave_status_t, &verifyHash_t, &verifyView, &validations_t, &signs_t);
	}
	else
	{
		generalRep.updateCheckpoint(verifyHash, verifyView, validations, signs);
	}
}

Validation ResiBFT::checkBlock(Justification justification, bool isFail)
{
	Validation validation = Validation();
	if (this->validations[this->view].isAccepted())
	{
		if (this->amTrustedReplicaIds())
		{
			Justification_t justification_t;
			setJustification(justification, &justification_t);
			Validation_t validation_t;
			sgx_status_t enclave_status_t;
			sgx_status_t ecall_status_t;
			ecall_status_t = TEE_checkBlock(global_eid, &enclave_status_t, &justification_t, &isFail, &validation_t);
			validation = getValidation(&validation_t);
		}
		else
		{
			validation = generalRep.checkBlock(this->nodes, justification, isFail);
		}
	}

	return validation;
}

// Receive messages
void ResiBFT::receiveMsgStart(MsgStart msgStart, const ClientNet::conn_t &conn)
{
	ClientID clientId = msgStart.clientId;
	if (this->clients.find(clientId) == this->clients.end())
	{
		(this->clients)[clientId] = std::make_tuple(true, 0, 0, conn);
	}
	if (!this->started)
	{
		this->started = true;
		this->getStartedCommon();
	}
}

void ResiBFT::receiveMsgTransaction(MsgTransaction msgTransaction, const ClientNet::conn_t &conn)
{
	this->handleMsgTransaction(msgTransaction);
}

void ResiBFT::receiveMsgNewviewCommon(MsgNewviewCommon msgNewviewCommon, const PeerNet::conn_t &conn)
{
	this->handleMsgNewviewCommon(msgNewviewCommon);
}

void ResiBFT::receiveMsgLdrprepareCommon(MsgLdrprepareCommon msgLdrprepareCommon, const PeerNet::conn_t &conn)
{
	this->handleMsgLdrprepareCommon(msgLdrprepareCommon);
}

void ResiBFT::receiveMsgPrepareCommon(MsgPrepareCommon msgPrepareCommon, const PeerNet::conn_t &conn)
{
	this->handleMsgPrepareCommon(msgPrepareCommon);
}

void ResiBFT::receiveMsgPrecommitCommon(MsgPrecommitCommon msgPrecommitCommon, const PeerNet::conn_t &conn)
{
	this->handleMsgPrecommitCommon(msgPrecommitCommon);
}

void ResiBFT::receiveMsgCommitCommon(MsgCommitCommon msgCommitCommon, const PeerNet::conn_t &conn)
{
	this->handleMsgCommitCommon(msgCommitCommon);
}

void ResiBFT::receiveMsgNewviewFast(MsgNewviewFast msgNewviewFast, const PeerNet::conn_t &conn)
{
	this->handleMsgNewviewFast(msgNewviewFast);
}

void ResiBFT::receiveMsgLdrprepareFast(MsgLdrprepareFast msgLdrprepareFast, const PeerNet::conn_t &conn)
{
	this->handleMsgLdrprepareFast(msgLdrprepareFast);
}

void ResiBFT::receiveMsgPrepareFast(MsgPrepareFast msgPrepareFast, const PeerNet::conn_t &conn)
{
	this->handleMsgPrepareFast(msgPrepareFast);
}

void ResiBFT::receiveMsgPrecommitFast(MsgPrecommitFast msgPrecommitFast, const PeerNet::conn_t &conn)
{
	this->handleMsgPrecommitFast(msgPrecommitFast);
}

void ResiBFT::receiveMsgValidationFast(MsgValidationFast msgValidationFast, const PeerNet::conn_t &conn)
{
	this->handleMsgValidationFast(msgValidationFast);
}

void ResiBFT::receiveMsgLdrprepareFast2Common(MsgLdrprepareFast2Common msgLdrprepareFast2Common, const PeerNet::conn_t &conn)
{
	this->handleMsgLdrprepareFast2Common(msgLdrprepareFast2Common);
}

void ResiBFT::receiveMsgPrepareFast2Common(MsgPrepareFast2Common msgPrepareFast2Common, const PeerNet::conn_t &conn)
{
	this->handleMsgPrepareFast2Common(msgPrepareFast2Common);
}

void ResiBFT::receiveMsgPrecommitFast2Common(MsgPrecommitFast2Common msgPrecommitFast2Common, const PeerNet::conn_t &conn)
{
	this->handleMsgPrecommitFast2Common(msgPrecommitFast2Common);
}

void ResiBFT::receiveMsgCommitFast2Common(MsgCommitFast2Common msgCommitFast2Common, const PeerNet::conn_t &conn)
{
	this->handleMsgCommitFast2Common(msgCommitFast2Common);
}

// Send messages
void ResiBFT::sendMsgNewviewCommon(MsgNewviewCommon msgNewviewCommon, Peers recipients)
{
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sending: " << msgNewviewCommon.toPrint() << " -> " << this->recipients2string(recipients) << COLOUR_NORMAL << std::endl;
	}
	this->peerNet.multicast_msg(msgNewviewCommon, getPeerIds(recipients));
	if (DEBUG_TIME)
	{
		this->printNowTime("Sending MsgNewview in common path");
	}
}

void ResiBFT::sendMsgLdrprepareCommon(MsgLdrprepareCommon msgLdrprepareCommon, Peers recipients)
{
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sending: " << msgLdrprepareCommon.toPrint() << " -> " << this->recipients2string(recipients) << COLOUR_NORMAL << std::endl;
	}
	this->peerNet.multicast_msg(msgLdrprepareCommon, getPeerIds(recipients));
	if (DEBUG_TIME)
	{
		this->printNowTime("Sending MsgLdrprepare in common path");
	}
}

void ResiBFT::sendMsgPrepareCommon(MsgPrepareCommon msgPrepareCommon, Peers recipients)
{
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sending: " << msgPrepareCommon.toPrint() << " -> " << this->recipients2string(recipients) << COLOUR_NORMAL << std::endl;
	}
	this->peerNet.multicast_msg(msgPrepareCommon, getPeerIds(recipients));
	if (DEBUG_TIME)
	{
		this->printNowTime("Sending MsgPrepare in common path");
	}
}

void ResiBFT::sendMsgPrecommitCommon(MsgPrecommitCommon msgPrecommitCommon, Peers recipients)
{
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sending: " << msgPrecommitCommon.toPrint() << " -> " << this->recipients2string(recipients) << COLOUR_NORMAL << std::endl;
	}
	this->peerNet.multicast_msg(msgPrecommitCommon, getPeerIds(recipients));
	if (DEBUG_TIME)
	{
		this->printNowTime("Sending MsgPrecommit in common path");
	}
}

void ResiBFT::sendMsgCommitCommon(MsgCommitCommon msgCommitCommon, Peers recipients)
{
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sending: " << msgCommitCommon.toPrint() << " -> " << this->recipients2string(recipients) << COLOUR_NORMAL << std::endl;
	}
	this->peerNet.multicast_msg(msgCommitCommon, getPeerIds(recipients));
	if (DEBUG_TIME)
	{
		this->printNowTime("Sending MsgCommit in common path");
	}
}

void ResiBFT::sendMsgNewviewFast(MsgNewviewFast msgNewviewFast, Peers recipients)
{
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sending: " << msgNewviewFast.toPrint() << " -> " << this->recipients2string(recipients) << COLOUR_NORMAL << std::endl;
	}
	this->peerNet.multicast_msg(msgNewviewFast, getPeerIds(recipients));
	if (DEBUG_TIME)
	{
		this->printNowTime("Sending MsgNewview in fast path");
	}
}

void ResiBFT::sendMsgLdrprepareFast(MsgLdrprepareFast msgLdrprepareFast, Peers recipients)
{
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sending: " << msgLdrprepareFast.toPrint() << " -> " << this->recipients2string(recipients) << COLOUR_NORMAL << std::endl;
	}
	this->peerNet.multicast_msg(msgLdrprepareFast, getPeerIds(recipients));
	if (DEBUG_TIME)
	{
		this->printNowTime("Sending MsgLdrprepare in fast path");
	}
}

void ResiBFT::sendMsgPrepareFast(MsgPrepareFast msgPrepareFast, Peers recipients)
{
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sending: " << msgPrepareFast.toPrint() << " -> " << this->recipients2string(recipients) << COLOUR_NORMAL << std::endl;
	}
	this->peerNet.multicast_msg(msgPrepareFast, getPeerIds(recipients));
	if (DEBUG_TIME)
	{
		this->printNowTime("Sending MsgPrepare in fast path");
	}
}

void ResiBFT::sendMsgPrecommitFast(MsgPrecommitFast msgPrecommitFast, Peers recipients)
{
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sending: " << msgPrecommitFast.toPrint() << " -> " << this->recipients2string(recipients) << COLOUR_NORMAL << std::endl;
	}
	this->peerNet.multicast_msg(msgPrecommitFast, getPeerIds(recipients));
	if (DEBUG_TIME)
	{
		this->printNowTime("Sending MsgPrecommit in fast path");
	}
}

void ResiBFT::sendMsgValidationFast(MsgValidationFast msgValidationFast, Peers recipients)
{
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sending: " << msgValidationFast.toPrint() << " -> " << this->recipients2string(recipients) << COLOUR_NORMAL << std::endl;
	}
	this->peerNet.multicast_msg(msgValidationFast, getPeerIds(recipients));
	if (DEBUG_TIME)
	{
		this->printNowTime("Sending MsgValidation in fast path");
	}
}

void ResiBFT::sendMsgLdrprepareFast2Common(MsgLdrprepareFast2Common msgLdrprepareFast2Common, Peers recipients)
{
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sending: " << msgLdrprepareFast2Common.toPrint() << " -> " << this->recipients2string(recipients) << COLOUR_NORMAL << std::endl;
	}
	this->peerNet.multicast_msg(msgLdrprepareFast2Common, getPeerIds(recipients));
	if (DEBUG_TIME)
	{
		this->printNowTime("Sending MsgLdrprepare for fast path to common path");
	}
}

void ResiBFT::sendMsgPrepareFast2Common(MsgPrepareFast2Common msgPrepareFast2Common, Peers recipients)
{
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sending: " << msgPrepareFast2Common.toPrint() << " -> " << this->recipients2string(recipients) << COLOUR_NORMAL << std::endl;
	}
	this->peerNet.multicast_msg(msgPrepareFast2Common, getPeerIds(recipients));
	if (DEBUG_TIME)
	{
		this->printNowTime("Sending MsgPrepare for fast path to common path");
	}
}

void ResiBFT::sendMsgPrecommitFast2Common(MsgPrecommitFast2Common msgPrecommitFast2Common, Peers recipients)
{
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sending: " << msgPrecommitFast2Common.toPrint() << " -> " << this->recipients2string(recipients) << COLOUR_NORMAL << std::endl;
	}
	this->peerNet.multicast_msg(msgPrecommitFast2Common, getPeerIds(recipients));
	if (DEBUG_TIME)
	{
		this->printNowTime("Sending MsgPrecommit for fast path to common path");
	}
}

void ResiBFT::sendMsgCommitFast2Common(MsgCommitFast2Common msgCommitFast2Common, Peers recipients)
{
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sending: " << msgCommitFast2Common.toPrint() << " -> " << this->recipients2string(recipients) << COLOUR_NORMAL << std::endl;
	}
	this->peerNet.multicast_msg(msgCommitFast2Common, getPeerIds(recipients));
	if (DEBUG_TIME)
	{
		this->printNowTime("Sending MsgCommit for fast path to common path");
	}
}

// Handle messages
void ResiBFT::handleMsgTransaction(MsgTransaction msgTransaction)
{
	std::lock_guard<std::mutex> guard(mutexTransaction);
	auto start = std::chrono::steady_clock::now();
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Handling MsgTransaction: " << msgTransaction.toPrint() << COLOUR_NORMAL << std::endl;
	}

	Transaction transaction = msgTransaction.transaction;
	ClientID clientId = transaction.getClientId();
	Clients::iterator it = this->clients.find(clientId);
	if (it != this->clients.end()) // Found an entry for [clientId]
	{
		ClientInformation clientInformation = it->second;
		bool running = std::get<0>(clientInformation);
		if (running)
		{
			// Got a new transaction from a live client
			if (this->transactions.size() < this->transactions.max_size())
			{
				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Pushing transaction: " << transaction.toPrint() << COLOUR_NORMAL << std::endl;
				}
				(this->clients)[clientId] = std::make_tuple(true, std::get<1>(clientInformation) + 1, std::get<2>(clientInformation), std::get<3>(clientInformation));
				this->transactions.push_back(transaction);
			}
			else
			{
				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Too many transactions (" << this->transactions.size() << "/" << this->transactions.max_size() << ")" << clientId << COLOUR_NORMAL << std::endl;
				}
			}
		}
		else
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Transaction rejected from stopped client: " << clientId << COLOUR_NORMAL << std::endl;
			}
		}
	}
	else
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Transaction rejected from unknown client: " << clientId << COLOUR_NORMAL << std::endl;
		}
	}

	auto end = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	statistics.addHandleTimes(this->view, time);
}

void ResiBFT::handleEarlierMessagesCommon()
{
	// Check if there are enough messages to start the next view
	if (this->amCurrentLeader())
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Leader handling earlier messages in common path" << COLOUR_NORMAL << std::endl;
		}
		Signs signs_MsgNewviewCommon = this->log.getMsgNewviewCommon(this->view, this->generalQuorumSize);
		if (signs_MsgNewviewCommon.getSize() == this->generalQuorumSize)
		{
			this->initiateMsgNewviewCommon();
		}
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Leader handled earlier messages in common path" << COLOUR_NORMAL << std::endl;
		}
	}
	else
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Replica handling earlier messages in common path" << COLOUR_NORMAL << std::endl;
		}

		// Check if the view has already been locked
		MsgPrecommitCommon msgPrecommitCommon = this->log.firstMsgPrecommitCommon(this->view);
		RoundData roundData_MsgPrecommitCommon = msgPrecommitCommon.roundData;
		Signs signs_MsgPrecommitCommon = msgPrecommitCommon.signs;
		Justification justification_MsgPrecommitCommon = Justification(roundData_MsgPrecommitCommon, signs_MsgPrecommitCommon);
		if (signs_MsgPrecommitCommon.getSize() == this->generalQuorumSize)
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Catching up using MsgPrecommit certificate in common path" << COLOUR_NORMAL << std::endl;
			}

			// Skip the prepare phase and pre-commit phase
			this->initializeMsgNewviewCommon();
			this->initializeMsgNewviewCommon();

			// Fill the block and check the committee
			MsgLdrprepareCommon msgLdrprepareCommon = this->log.firstMsgLdrprepareCommon(this->view);
			ProposalCommon proposalCommon_MsgLdrprepareCommon = msgLdrprepareCommon.proposalCommon;
			Committee committee_MsgLdrprepareCommon = msgLdrprepareCommon.committee;
			Block block = proposalCommon_MsgLdrprepareCommon.getBlock();
			this->blocks[this->view] = block;
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Fill the block with MsgLdrprepare" << COLOUR_NORMAL << std::endl;
			}

			if (committee_MsgLdrprepareCommon.isSet())
			{
				this->committee = committee_MsgLdrprepareCommon;
				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Update the committee" << COLOUR_NORMAL << std::endl;
				}
			}

			// Store [justification_MsgPrecommitCommon]
			this->respondMsgPrecommitCommon(justification_MsgPrecommitCommon);

			MsgCommitCommon msgCommitCommon = this->log.firstMsgCommitCommon(this->view);
			RoundData roundData_MsgCommitCommon = msgCommitCommon.roundData;
			Signs signs_MsgCommitCommon = msgCommitCommon.signs;
			Justification justification_MsgCommitCommon = Justification(roundData_MsgCommitCommon, signs_MsgCommitCommon);
			if (signs_MsgCommitCommon.getSize() == this->generalQuorumSize)
			{
				this->executeBlockCommon(justification_MsgCommitCommon.getRoundData());
			}
		}
		else
		{
			MsgPrepareCommon msgPrepareCommon = this->log.firstMsgPrepareCommon(this->view);
			RoundData roundData_MsgPrepareCommon = msgPrepareCommon.roundData;
			Signs signs_MsgPrepareCommon = msgPrepareCommon.signs;
			Justification justification_MsgPrepareCommon = Justification(roundData_MsgPrepareCommon, signs_MsgPrepareCommon);
			if (signs_MsgPrepareCommon.getSize() == this->generalQuorumSize)
			{
				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Catching up using MsgPrepare certificate in common path" << COLOUR_NORMAL << std::endl;
				}

				// Skip the prepare phase
				this->initializeMsgNewviewCommon();

				// Store [justification_MsgPrepareCommon]
				this->respondMsgPrepareCommon(justification_MsgPrepareCommon);

				// Fill the block and check the committee
				MsgLdrprepareCommon msgLdrprepareCommon = this->log.firstMsgLdrprepareCommon(this->view);
				ProposalCommon proposalCommon_MsgLdrprepareCommon = msgLdrprepareCommon.proposalCommon;
				Committee committee_MsgLdrprepareCommon = msgLdrprepareCommon.committee;
				Block block = proposalCommon_MsgLdrprepareCommon.getBlock();
				this->blocks[this->view] = block;
				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Fill the block with MsgLdrprepare" << COLOUR_NORMAL << std::endl;
				}

				if (committee_MsgLdrprepareCommon.isSet())
				{
					this->committee = committee_MsgLdrprepareCommon;
					if (DEBUG_HELP)
					{
						std::cout << COLOUR_BLUE << this->printReplicaId() << "Update the committee" << COLOUR_NORMAL << std::endl;
					}
				}
			}
			else
			{
				MsgLdrprepareCommon msgLdrprepareCommon = this->log.firstMsgLdrprepareCommon(this->view);
				Signs signs_MsgLdrprepareCommon = msgLdrprepareCommon.signs;

				// Check if the proposal has been stored
				if (signs_MsgLdrprepareCommon.getSize() == 1)
				{
					if (DEBUG_HELP)
					{
						std::cout << COLOUR_BLUE << this->printReplicaId() << "Catching up using MsgLdrprepare proposal in common path" << COLOUR_NORMAL << std::endl;
					}
					ProposalCommon proposalCommon_MsgLdrprepareCommon = msgLdrprepareCommon.proposalCommon;
					Committee committee_MsgLdrprepareCommon = msgLdrprepareCommon.committee;
					Block block = proposalCommon_MsgLdrprepareCommon.getBlock();
					Justification justification_MsgLdrprepareCommon = proposalCommon_MsgLdrprepareCommon.getJustification();
					if (committee_MsgLdrprepareCommon.isSet())
					{
						this->committee = committee_MsgLdrprepareCommon;
					}
					this->respondMsgLdrprepareCommon(justification_MsgLdrprepareCommon, committee_MsgLdrprepareCommon, block);
				}
			}
		}
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Replica handled earlier messages in common path" << COLOUR_NORMAL << std::endl;
		}
	}
}

void ResiBFT::handleEarlierMessagesFast()
{
	// Check if there are enough messages to start the next view
	if (this->amCurrentLeader())
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Leader handling earlier messages in fast path" << COLOUR_NORMAL << std::endl;
		}
		std::set<MsgNewviewFast> msgNewviewFasts = this->log.getMsgNewviewFast(this->view, this->trustedQuorumSize);
		if (msgNewviewFasts.size() == this->trustedQuorumSize)
		{
			this->initiateMsgNewviewFast();
		}
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Leader handled earlier messages in fast path" << COLOUR_NORMAL << std::endl;
		}
	}
	else
	{
		if (this->amCommitteeReplicaIds())
		{
			MsgLdrprepareFast msgLdrprepareFast = this->log.firstMsgLdrprepareFast(this->view);
			ProposalFast proposalFast_MsgLdrprepareFast = msgLdrprepareFast.proposalFast;
			Accumulator accumulator_MsgLdrprepareFast = proposalFast_MsgLdrprepareFast.getAccumulator();
			View proposeView_MsgLdrprepareFast = accumulator_MsgLdrprepareFast.getProposeView();
			if (proposeView_MsgLdrprepareFast == this->view)
			{
				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Committee member handling earlier messages in fast path" << COLOUR_NORMAL << std::endl;
				}

				// Check if the view has already been entered in Precommit phase
				MsgPrecommitFast msgPrecommitFast = this->log.firstMsgPrecommitFast(this->view);
				RoundData roundData_MsgPrecommitFast = msgPrecommitFast.roundData;
				Signs signs_MsgPrecommitFast = msgPrecommitFast.signs;
				Justification justification_MsgPrecommitFast = Justification(roundData_MsgPrecommitFast, signs_MsgPrecommitFast);
				if (signs_MsgPrecommitFast.getSize() == this->trustedQuorumSize)
				{
					if (DEBUG_HELP)
					{
						std::cout << COLOUR_BLUE << this->printReplicaId() << "Catching up using MsgPrecommit certificate in fast path" << COLOUR_NORMAL << std::endl;
					}

					// Skip the prepare phase and pre-commit phase
					this->initializeMsgNewviewFast();
					this->initializeMsgNewviewFast();

					// Fill the block
					if (DEBUG_HELP)
					{
						std::cout << COLOUR_BLUE << this->printReplicaId() << "Fill the block with MsgLdrprepare proposal" << COLOUR_NORMAL << std::endl;
					}
					MsgLdrprepareFast msgLdrprepareFast = this->log.firstMsgLdrprepareFast(this->view);
					if (msgLdrprepareFast.signs.getSize() == 1)
					{
						Validations validations_MsgLdrprepareFast = msgLdrprepareFast.validations;
						this->validations[this->view] = validations_MsgLdrprepareFast;
						ProposalFast proposalFast_MsgLdrprepareFast = msgLdrprepareFast.proposalFast;
						Block block = proposalFast_MsgLdrprepareFast.getBlock();
						this->blocks[this->view] = block;
					}

					// Execute the block
					bool isFail_MsgPrecommitFast = msgPrecommitFast.isFail;
					Validation validation_MsgPrecommitFast = Validation();
					if (signs_MsgPrecommitFast.getSize() == this->trustedQuorumSize && !isFail_MsgPrecommitFast && this->verifyJustification(justification_MsgPrecommitFast))
					{
						validation_MsgPrecommitFast = this->checkBlock(justification_MsgPrecommitFast, isFail_MsgPrecommitFast);
						if (DEBUG_HELP)
						{
							std::cout << COLOUR_BLUE << this->printReplicaId() << "Checking MsgPrecommit in fast path by earlier messages: " << validation_MsgPrecommitFast.toPrint() << COLOUR_NORMAL << std::endl;
						}

						if (validation_MsgPrecommitFast.isAccepted())
						{
							Hash verifyHash_Checkpoint = this->blocks[this->view].getPreviousHash();
							View verifyView_Checkpoint = this->view - 1;
							Validations validations_Checkpoint = this->validations[this->view];
							Signs signs_Checkpoint = signs_MsgPrecommitFast;
							this->updateCheckpoint(verifyHash_Checkpoint, verifyView_Checkpoint, validations_Checkpoint, signs_Checkpoint);
						}
					}
					else
					{
						validation_MsgPrecommitFast = this->checkBlock(justification_MsgPrecommitFast, isFail_MsgPrecommitFast);
					}
					this->executeBlockFast(roundData_MsgPrecommitFast, validation_MsgPrecommitFast);
				}
				else
				{
					MsgPrepareFast msgPrepareFast = this->log.firstMsgPrepareFast(this->view);
					RoundData roundData_MsgPrepareFast = msgPrepareFast.roundData;
					Signs signs_MsgPrepareFast = msgPrepareFast.signs;
					Justification justification_MsgPrepareFast = Justification(roundData_MsgPrepareFast, signs_MsgPrepareFast);
					if (signs_MsgPrepareFast.getSize() == this->trustedQuorumSize)
					{
						if (DEBUG_HELP)
						{
							std::cout << COLOUR_BLUE << this->printReplicaId() << "Catching up using MsgPrepare certificate in fast path" << COLOUR_NORMAL << std::endl;
						}
						bool isFail_MsgPrepareFast = msgPrepareFast.isFail;

						// Skip the prepare phase
						this->initializeMsgNewviewFast();

						// Fill the block
						if (DEBUG_HELP)
						{
							std::cout << COLOUR_BLUE << this->printReplicaId() << "Fill the block with MsgLdrprepare proposal in fast path" << COLOUR_NORMAL << std::endl;
						}
						MsgLdrprepareFast msgLdrprepareFast = this->log.firstMsgLdrprepareFast(this->view);
						if (msgLdrprepareFast.signs.getSize() == 1)
						{
							Validations validations_MsgLdrprepareFast = msgLdrprepareFast.validations;
							this->validations[this->view] = validations_MsgLdrprepareFast;
							ProposalFast proposalFast_MsgLdrprepareFast = msgLdrprepareFast.proposalFast;
							Block block = proposalFast_MsgLdrprepareFast.getBlock();
							this->blocks[this->view] = block;
						}

						// Store [justification_MsgPrepareFast]
						this->respondMsgPrepareFast(justification_MsgPrepareFast, isFail_MsgPrepareFast);
					}
					else
					{
						MsgLdrprepareFast msgLdrprepareFast = this->log.firstMsgLdrprepareFast(this->view);

						// Check if the proposal has been stored
						if (msgLdrprepareFast.signs.getSize() == 1)
						{
							if (DEBUG_HELP)
							{
								std::cout << COLOUR_BLUE << this->printReplicaId() << "Catching up using MsgLdrprepare proposal in fast path" << COLOUR_NORMAL << std::endl;
							}
							ProposalFast proposalFast_MsgLdrprepareFast = msgLdrprepareFast.proposalFast;
							Validations validations_MsgLdrprepareFast = msgLdrprepareFast.validations;
							Accumulator accumulator_MsgLdrprepareFast = proposalFast_MsgLdrprepareFast.getAccumulator();
							Block block = proposalFast_MsgLdrprepareFast.getBlock();
							this->respondMsgLdrprepareFast(accumulator_MsgLdrprepareFast, validations_MsgLdrprepareFast, block);
						}
					}
				}

				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Committee member handled earlier messages in fast path" << COLOUR_NORMAL << std::endl;
				}
			}
		}
		else
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Replica handling earlier messages in fast path" << COLOUR_NORMAL << std::endl;
			}

			MsgValidationFast msgValidationFast = this->log.firstMsgValidationFast(this->view);
			RoundData roundData_MsgValidationFast = msgValidationFast.roundData;
			View proposeView_MsgValidationFast = roundData_MsgValidationFast.getProposeView();
			if (proposeView_MsgValidationFast == this->view)
			{
				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Catching up using MsgValidation certificate in fast path" << COLOUR_NORMAL << std::endl;
				}

				Validations validations_MsgValidationFast = msgValidationFast.validations;
				Signs signs_MsgValidationFast = msgValidationFast.signs;
				Phase phase_MsgValidationFast = roundData_MsgValidationFast.getPhase();
				Block block = msgValidationFast.block;
				Justification justification_MsgValidationFast = Justification(roundData_MsgValidationFast, signs_MsgValidationFast);

				if (signs_MsgValidationFast.getSize() == this->trustedQuorumSize && this->verifyJustification(justification_MsgValidationFast))
				{
					this->blocks[this->view] = block;
					this->validations[this->view] = validations_MsgValidationFast;
					bool isFail_MsgValidationFast = validations_MsgValidationFast.isAccepted();

					Validation validation_MsgValidationFast = this->checkBlock(justification_MsgValidationFast, isFail_MsgValidationFast);
					if (DEBUG_HELP)
					{
						std::cout << COLOUR_BLUE << this->printReplicaId() << "Checking MsgValidation in fast path: " << validation_MsgValidationFast.toPrint() << COLOUR_NORMAL << std::endl;
					}

					if (validation_MsgValidationFast.isAccepted())
					{
						Hash verifyHash_Checkpoint = this->blocks[this->view].getPreviousHash();
						View verifyView_Checkpoint = this->view - 1;
						Validations validations_Checkpoint = this->validations[this->view];
						Signs signs_Checkpoint = signs_MsgValidationFast;
						this->updateCheckpoint(verifyHash_Checkpoint, verifyView_Checkpoint, validations_Checkpoint, signs_Checkpoint);
					}

					this->executeBlockFast(roundData_MsgValidationFast, validation_MsgValidationFast);
				}
			}
			else
			{
				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Replica handling earlier messages for fast path to common path" << COLOUR_NORMAL << std::endl;
				}

				MsgLdrprepareFast2Common msgLdrprepareFast2Common = this->log.firstMsgLdrprepareFast2Common(this->view);
				ProposalCommon proposalCommon_MsgLdrprepareFast2Common = msgLdrprepareFast2Common.proposalCommon;
				Justification justification_MsgLdrprepareFast2Common = proposalCommon_MsgLdrprepareFast2Common.getJustification();
				View proposeView_MsgLdrprepareFast2Common = justification_MsgLdrprepareFast2Common.getRoundData().getProposeView();
				if (proposeView_MsgLdrprepareFast2Common == this->view)
				{
					// Check if the view has already been locked
					MsgPrecommitFast2Common msgPrecommitFast2Common = this->log.firstMsgPrecommitFast2Common(this->view);
					RoundData roundData_MsgPrecommitFast2Common = msgPrecommitFast2Common.roundData;
					Signs signs_MsgPrecommitFast2Common = msgPrecommitFast2Common.signs;
					Justification justification_MsgPrecommitFast2Common = Justification(roundData_MsgPrecommitFast2Common, signs_MsgPrecommitFast2Common);
					if (signs_MsgPrecommitFast2Common.getSize() == this->generalQuorumSize)
					{
						if (DEBUG_HELP)
						{
							std::cout << COLOUR_BLUE << this->printReplicaId() << "Catching up using MsgPrecommit certificate for fast path to common path" << COLOUR_NORMAL << std::endl;
						}

						// Skip the prepare phase and pre-commit phase
						this->initializeMsgNewviewCommon();
						this->initializeMsgNewviewCommon();

						// Fill the block and check the committee
						Committee committee_MsgLdrprepareFast2Common = msgLdrprepareFast2Common.committee;
						Block block = proposalCommon_MsgLdrprepareFast2Common.getBlock();
						this->blocks[this->view] = block;
						if (DEBUG_HELP)
						{
							std::cout << COLOUR_BLUE << this->printReplicaId() << "Fill the block with MsgLdrprepare" << COLOUR_NORMAL << std::endl;
						}

						if (committee_MsgLdrprepareFast2Common.isSet())
						{
							this->committee = committee_MsgLdrprepareFast2Common;
							if (DEBUG_HELP)
							{
								std::cout << COLOUR_BLUE << this->printReplicaId() << "Update the committee" << COLOUR_NORMAL << std::endl;
							}
						}

						// Store [justification_MsgPrecommitFast2Common]
						this->respondMsgPrecommitFast2Common(justification_MsgPrecommitFast2Common);

						MsgCommitFast2Common msgCommitFast2Common = this->log.firstMsgCommitFast2Common(this->view);
						RoundData roundData_MsgCommitFast2Common = msgCommitFast2Common.roundData;
						Signs signs_MsgCommitFast2Common = msgCommitFast2Common.signs;
						Justification justification_MsgCommitFast2Common = Justification(roundData_MsgCommitFast2Common, signs_MsgCommitFast2Common);
						if (signs_MsgCommitFast2Common.getSize() == this->generalQuorumSize)
						{
							this->executeBlockCommon(justification_MsgCommitFast2Common.getRoundData());
						}
					}
					else
					{
						MsgPrepareFast2Common msgPrepareFast2Common = this->log.firstMsgPrepareFast2Common(this->view);
						RoundData roundData_MsgPrepareFast2Common = msgPrepareFast2Common.roundData;
						Signs signs_MsgPrepareFast2Common = msgPrepareFast2Common.signs;
						Justification justification_MsgPrepareFast2Common = Justification(roundData_MsgPrepareFast2Common, signs_MsgPrepareFast2Common);
						if (signs_MsgPrepareFast2Common.getSize() == this->generalQuorumSize)
						{
							if (DEBUG_HELP)
							{
								std::cout << COLOUR_BLUE << this->printReplicaId() << "Catching up using MsgPrepare certificate for fast path to common path" << COLOUR_NORMAL << std::endl;
							}

							// Skip the prepare phase
							this->initializeMsgNewviewCommon();

							// Store [justification_MsgPrepareFast2Common]
							this->respondMsgPrepareFast2Common(justification_MsgPrepareFast2Common);

							// Fill the block and check the committee
							Committee committee_MsgLdrprepareFast2Common = msgLdrprepareFast2Common.committee;
							Block block = proposalCommon_MsgLdrprepareFast2Common.getBlock();
							this->blocks[this->view] = block;
							if (DEBUG_HELP)
							{
								std::cout << COLOUR_BLUE << this->printReplicaId() << "Fill the block with MsgLdrprepare for fast path to common path" << COLOUR_NORMAL << std::endl;
							}

							if (committee_MsgLdrprepareFast2Common.isSet())
							{
								this->committee = committee_MsgLdrprepareFast2Common;
								if (DEBUG_HELP)
								{
									std::cout << COLOUR_BLUE << this->printReplicaId() << "Update the committee" << COLOUR_NORMAL << std::endl;
								}
							}
						}
						else
						{
							Signs signs_MsgLdrprepareFast2Common = msgLdrprepareFast2Common.signs;

							// Check if the proposal has been stored
							if (signs_MsgLdrprepareFast2Common.getSize() == 1)
							{
								if (DEBUG_HELP)
								{
									std::cout << COLOUR_BLUE << this->printReplicaId() << "Catching up using MsgLdrprepare proposal for fast path to common path" << COLOUR_NORMAL << std::endl;
								}
								Committee committee_MsgLdrprepareFast2Common = msgLdrprepareFast2Common.committee;
								Block block = proposalCommon_MsgLdrprepareFast2Common.getBlock();
								Justification justification_MsgLdrprepareFast2Common = proposalCommon_MsgLdrprepareFast2Common.getJustification();
								if (committee_MsgLdrprepareFast2Common.isSet())
								{
									this->committee = committee_MsgLdrprepareFast2Common;
								}
								this->respondMsgLdrprepareFast2Common(justification_MsgLdrprepareFast2Common, committee_MsgLdrprepareFast2Common, block);
							}
						}
					}
				}

				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Replica handled earlier messages for fast path to common path" << COLOUR_NORMAL << std::endl;
				}
			}

			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Replica handled earlier messages in fast path" << COLOUR_NORMAL << std::endl;
			}
		}
	}
}

void ResiBFT::handleMsgNewviewCommon(MsgNewviewCommon msgNewviewCommon)
{
	auto start = std::chrono::steady_clock::now();

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Handling MsgNewview in common path: " << msgNewviewCommon.toPrint() << COLOUR_NORMAL << std::endl;
	}
	RoundData roundData_MsgNewviewCommon = msgNewviewCommon.roundData;
	Hash proposeHash_MsgNewviewCommon = roundData_MsgNewviewCommon.getProposeHash();
	View proposeView_MsgNewviewCommon = roundData_MsgNewviewCommon.getProposeView();
	Phase phase_MsgNewviewCommon = roundData_MsgNewviewCommon.getPhase();

	if (proposeHash_MsgNewviewCommon.isDummy() && proposeView_MsgNewviewCommon >= this->view && phase_MsgNewviewCommon == PHASE_NEWVIEW_COMMON)
	{
		if (proposeView_MsgNewviewCommon == this->view)
		{
			if (this->log.storeMsgNewviewCommon(msgNewviewCommon) == this->generalQuorumSize)
			{
				this->initiateMsgNewviewCommon();
			}
		}
		else
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing MsgNewview in common path: " << msgNewviewCommon.toPrint() << COLOUR_NORMAL << std::endl;
			}
			this->log.storeMsgNewviewCommon(msgNewviewCommon);
		}
	}

	auto end = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	statistics.addHandleTimes(this->view, time);
}

void ResiBFT::handleMsgLdrprepareCommon(MsgLdrprepareCommon MsgLdrprepareCommon)
{
	auto start = std::chrono::steady_clock::now();

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Handling MsgLdrprepare in common path: " << MsgLdrprepareCommon.toPrint() << COLOUR_NORMAL << std::endl;
	}
	ProposalCommon proposalCommon_MsgLdrprepareCommon = MsgLdrprepareCommon.proposalCommon;
	Committee committee_MsgLdrprepareCommon = MsgLdrprepareCommon.committee;
	Signs signs_MsgLdrprepareCommon = MsgLdrprepareCommon.signs;
	Justification justification_MsgNewviewCommon = proposalCommon_MsgLdrprepareCommon.getJustification();
	RoundData roundData_MsgNewviewCommon = justification_MsgNewviewCommon.getRoundData();
	View proposeView_MsgNewviewCommon = roundData_MsgNewviewCommon.getProposeView();
	Hash justifyHash_MsgNewviewCommon = roundData_MsgNewviewCommon.getJustifyHash();
	Block block = proposalCommon_MsgLdrprepareCommon.getBlock();

	// Verify the [signs_MsgLdrprepareCommon] in [MsgLdrprepareCommon]
	if (this->verifyProposalCommon(proposalCommon_MsgLdrprepareCommon, signs_MsgLdrprepareCommon) && block.extends(justifyHash_MsgNewviewCommon) && proposeView_MsgNewviewCommon >= this->view)
	{
		if (proposeView_MsgNewviewCommon == this->view)
		{
			if (this->path == COMMON_PATH)
			{
				this->respondMsgLdrprepareCommon(justification_MsgNewviewCommon, committee_MsgLdrprepareCommon, block);
			}
			else
			{
				if (DEBUG_BASIC)
				{
					std::cout << COLOUR_RED << this->printReplicaId() << "Fail to handle MsgLdrprepare in common path" << COLOUR_NORMAL << std::endl;
				}
			}
		}
		else
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing MsgLdrprepare in common path: " << MsgLdrprepareCommon.toPrint() << COLOUR_NORMAL << std::endl;
			}
			this->log.storeMsgLdrprepareCommon(MsgLdrprepareCommon);
		}
	}

	auto end = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	statistics.addHandleTimes(this->view, time);
}

void ResiBFT::handleMsgPrepareCommon(MsgPrepareCommon msgPrepareCommon)
{
	auto start = std::chrono::steady_clock::now();

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Handling MsgPrepare in common path: " << msgPrepareCommon.toPrint() << COLOUR_NORMAL << std::endl;
	}
	RoundData roundData_MsgPrepareCommon = msgPrepareCommon.roundData;
	Signs signs_MsgPrepare = msgPrepareCommon.signs;
	View proposeView_MsgPrepareCommon = roundData_MsgPrepareCommon.getProposeView();
	Phase phase_MsgPrepareCommon = roundData_MsgPrepareCommon.getPhase();
	Justification justification_MsgPrepareCommon = Justification(roundData_MsgPrepareCommon, signs_MsgPrepare);

	if (proposeView_MsgPrepareCommon >= this->view && phase_MsgPrepareCommon == PHASE_PREPARE_COMMON)
	{
		if (proposeView_MsgPrepareCommon == this->view)
		{
			if (this->path == COMMON_PATH)
			{
				if (this->amCurrentLeader())
				{
					if (this->log.storeMsgPrepareCommon(msgPrepareCommon) == this->generalQuorumSize)
					{
						this->initiateMsgPrepareCommon(roundData_MsgPrepareCommon);
					}
				}
				else
				{
					if (signs_MsgPrepare.getSize() == this->generalQuorumSize)
					{
						this->respondMsgPrepareCommon(justification_MsgPrepareCommon);
					}
				}
			}
			else
			{
				if (DEBUG_BASIC)
				{
					std::cout << COLOUR_RED << this->printReplicaId() << "Fail to handle MsgPrepare in common path" << COLOUR_NORMAL << std::endl;
				}
			}
		}
		else
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing MsgPrepare in common path: " << msgPrepareCommon.toPrint() << COLOUR_NORMAL << std::endl;
			}
			this->log.storeMsgPrepareCommon(msgPrepareCommon);
		}
	}

	auto end = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	statistics.addHandleTimes(this->view, time);
}

void ResiBFT::handleMsgPrecommitCommon(MsgPrecommitCommon msgPrecommitCommon)
{
	auto start = std::chrono::steady_clock::now();

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Handling MsgPrecommit in common path: " << msgPrecommitCommon.toPrint() << COLOUR_NORMAL << std::endl;
	}
	RoundData roundData_MsgPrecommitCommon = msgPrecommitCommon.roundData;
	Signs signs_MsgPrecommitCommon = msgPrecommitCommon.signs;
	View proposeView_MsgPrecommitCommon = roundData_MsgPrecommitCommon.getProposeView();
	Phase phase_MsgPrecommitCommon = roundData_MsgPrecommitCommon.getPhase();
	Justification justification_MsgPrecommitCommon = Justification(roundData_MsgPrecommitCommon, signs_MsgPrecommitCommon);

	if (proposeView_MsgPrecommitCommon >= this->view && phase_MsgPrecommitCommon == PHASE_PRECOMMIT_COMMON)
	{
		if (proposeView_MsgPrecommitCommon == this->view)
		{
			if (this->path == COMMON_PATH)
			{
				if (this->amCurrentLeader())
				{
					if (this->log.storeMsgPrecommitCommon(msgPrecommitCommon) == this->generalQuorumSize)
					{
						this->initiateMsgPrecommitCommon(roundData_MsgPrecommitCommon);
					}
				}
				else
				{
					if (signs_MsgPrecommitCommon.getSize() == this->generalQuorumSize)
					{
						this->respondMsgPrecommitCommon(justification_MsgPrecommitCommon);
					}
				}
			}
			else
			{
				if (DEBUG_BASIC)
				{
					std::cout << COLOUR_RED << this->printReplicaId() << "Fail to handle MsgPrepare in common path" << COLOUR_NORMAL << std::endl;
				}
			}
		}
		else
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing MsgPrecommit in common path: " << msgPrecommitCommon.toPrint() << COLOUR_NORMAL << std::endl;
			}
			this->log.storeMsgPrecommitCommon(msgPrecommitCommon);
		}
	}

	auto end = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	statistics.addHandleTimes(this->view, time);
}

void ResiBFT::handleMsgCommitCommon(MsgCommitCommon msgCommitCommon)
{
	auto start = std::chrono::steady_clock::now();

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Handling MsgCommit in common path: " << msgCommitCommon.toPrint() << COLOUR_NORMAL << std::endl;
	}
	RoundData roundData_MsgCommitCommon = msgCommitCommon.roundData;
	Signs signs_MsgCommitCommon = msgCommitCommon.signs;
	View proposeView_MsgCommitCommon = roundData_MsgCommitCommon.getProposeView();
	Phase phase_MsgCommitCommon = roundData_MsgCommitCommon.getPhase();
	Justification justification_MsgCommitCommon = Justification(roundData_MsgCommitCommon, signs_MsgCommitCommon);

	if (proposeView_MsgCommitCommon >= this->view && phase_MsgCommitCommon == PHASE_COMMIT_COMMON)
	{
		if (proposeView_MsgCommitCommon == this->view)
		{
			if (this->path == COMMON_PATH)
			{
				if (this->amCurrentLeader())
				{
					if (this->log.storeMsgCommitCommon(msgCommitCommon) == this->generalQuorumSize)
					{
						this->initiateMsgCommitCommon(roundData_MsgCommitCommon);
					}
				}
				else
				{
					if (signs_MsgCommitCommon.getSize() == this->generalQuorumSize)
					{
						this->executeBlockCommon(roundData_MsgCommitCommon);
					}
				}
			}
			else
			{
				if (DEBUG_BASIC)
				{
					std::cout << COLOUR_RED << this->printReplicaId() << "Fail to handle MsgCommit in common path" << COLOUR_NORMAL << std::endl;
				}
			}
		}
		else
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing MsgCommit in common path: " << msgCommitCommon.toPrint() << COLOUR_NORMAL << std::endl;
			}
			this->log.storeMsgCommitCommon(msgCommitCommon);
		}
	}

	auto end = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	statistics.addHandleTimes(this->view, time);
}

void ResiBFT::handleMsgNewviewFast(MsgNewviewFast msgNewviewFast)
{
	auto start = std::chrono::steady_clock::now();

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Handling MsgNewview in fast path: " << msgNewviewFast.toPrint() << COLOUR_NORMAL << std::endl;
	}
	RoundData roundData_MsgNewviewFast = msgNewviewFast.roundData;
	Hash proposeHash_MsgNewviewFast = roundData_MsgNewviewFast.getProposeHash();
	View proposeView_MsgNewviewFast = roundData_MsgNewviewFast.getProposeView();
	Phase phase_MsgNewviewFast = roundData_MsgNewviewFast.getPhase();

	if (proposeHash_MsgNewviewFast.isDummy() && proposeView_MsgNewviewFast >= this->view && phase_MsgNewviewFast == PHASE_NEWVIEW_FAST)
	{
		if (proposeView_MsgNewviewFast == this->view)
		{
			if (this->path == FAST_PATH)
			{
				if (this->log.storeMsgNewviewFast(msgNewviewFast) == this->generalQuorumSize)
				{
					this->initiateMsgNewviewFast();
				}
			}
			else
			{
				if (DEBUG_BASIC)
				{
					std::cout << COLOUR_RED << this->printReplicaId() << "Fail to handle MsgNewview in fast path" << COLOUR_NORMAL << std::endl;
				}
			}
		}
		else
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing MsgNewview in fast path: " << msgNewviewFast.toPrint() << COLOUR_NORMAL << std::endl;
			}
			this->log.storeMsgNewviewFast(msgNewviewFast);
		}
	}

	auto end = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	statistics.addHandleTimes(this->view, time);
}

void ResiBFT::handleMsgLdrprepareFast(MsgLdrprepareFast msgLdrprepareFast)
{
	auto start = std::chrono::steady_clock::now();

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Handling MsgLdrprepare in fast path: " << msgLdrprepareFast.toPrint() << COLOUR_NORMAL << std::endl;
	}
	ProposalFast proposal_msgLdrprepareFast = msgLdrprepareFast.proposalFast;
	Validations validations_msgLdrprepareFast = msgLdrprepareFast.validations;
	Signs signs_msgLdrprepareFast = msgLdrprepareFast.signs;
	Accumulator accumulator_msgLdrprepareFast = proposal_msgLdrprepareFast.getAccumulator();
	View proposeView_msgLdrprepareFast = accumulator_msgLdrprepareFast.getProposeView();
	Hash prepareHash_msgLdrprepareFast = accumulator_msgLdrprepareFast.getPrepareHash();
	Block block = proposal_msgLdrprepareFast.getBlock();

	// Verify the [signs_msgLdrprepareFast] in [msgLdrprepareFast]
	if (this->verifyProposalFast(proposal_msgLdrprepareFast, signs_msgLdrprepareFast) && block.extends(prepareHash_msgLdrprepareFast) && proposeView_msgLdrprepareFast >= this->view)
	{
		if (proposeView_msgLdrprepareFast == this->view)
		{
			if (this->path == FAST_PATH)
			{
				this->respondMsgLdrprepareFast(accumulator_msgLdrprepareFast, validations_msgLdrprepareFast, block);
			}
			else
			{
				if (DEBUG_BASIC)
				{
					std::cout << COLOUR_RED << this->printReplicaId() << "Fail to handle MsgLdrprepare in fast path" << COLOUR_NORMAL << std::endl;
				}
			}
		}
		else
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing MsgLdrprepare in fast path: " << msgLdrprepareFast.toPrint() << COLOUR_NORMAL << std::endl;
			}
			this->log.storeMsgLdrprepareFast(msgLdrprepareFast);
		}
	}

	auto end = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	statistics.addHandleTimes(this->view, time);
}

void ResiBFT::handleMsgPrepareFast(MsgPrepareFast msgPrepareFast)
{
	auto start = std::chrono::steady_clock::now();

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Handling MsgPrepare in fast path: " << msgPrepareFast.toPrint() << COLOUR_NORMAL << std::endl;
	}
	RoundData roundData_MsgPrepareFast = msgPrepareFast.roundData;
	bool isFail_MsgPrepareFast = msgPrepareFast.isFail;
	Signs signs_MsgPrepareFast = msgPrepareFast.signs;
	View proposeView_MsgPrepareFast = roundData_MsgPrepareFast.getProposeView();
	Phase phase_MsgPrepareFast = roundData_MsgPrepareFast.getPhase();
	Justification justification_MsgPrepareFast = Justification(roundData_MsgPrepareFast, signs_MsgPrepareFast);

	if (proposeView_MsgPrepareFast >= this->view && phase_MsgPrepareFast == PHASE_PREPARE_FAST)
	{
		if (proposeView_MsgPrepareFast == this->view)
		{
			if (this->path == FAST_PATH)
			{
				if (this->amCurrentLeader())
				{
					if (!this->amTrustfailReplicaIds())
					{
						if (this->log.storeMsgPrepareFast(msgPrepareFast) >= this->trustedQuorumSize && this->log.checkMsgPrepareFast(this->view, this->trustedQuorumSize) && !this->isHandleMsgPrepareFast)
						{
							this->initiateMsgPrepareFast(roundData_MsgPrepareFast);
							this->isHandleMsgPrepareFast = true;
						}
						else if (this->log.storeMsgPrepareFast(msgPrepareFast) == NUM_COMMITTEE_MEMBERS)
						{
							this->initiateMsgPrepareFast(roundData_MsgPrepareFast);
						}
					}
					else
					{
						if (this->log.storeMsgPrepareFast(msgPrepareFast) == this->trustedQuorumSize)
						{
							this->initiateMsgPrepareFast(roundData_MsgPrepareFast);
						}
					}
				}
				else
				{
					if (signs_MsgPrepareFast.getSize() == this->trustedQuorumSize)
					{
						this->respondMsgPrepareFast(justification_MsgPrepareFast, isFail_MsgPrepareFast);
					}
				}
			}
			else
			{
				if (DEBUG_BASIC)
				{
					std::cout << COLOUR_RED << this->printReplicaId() << "Fail to handle MsgPrepare in fast path" << COLOUR_NORMAL << std::endl;
				}
			}
		}
		else
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing MsgPrepare in fast path: " << msgPrepareFast.toPrint() << COLOUR_NORMAL << std::endl;
			}
			this->log.storeMsgPrepareFast(msgPrepareFast);
		}
	}

	auto end = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	statistics.addHandleTimes(this->view, time);
}

void ResiBFT::handleMsgPrecommitFast(MsgPrecommitFast msgPrecommitFast)
{
	auto start = std::chrono::steady_clock::now();

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Handling MsgPrecommit in fast path: " << msgPrecommitFast.toPrint() << COLOUR_NORMAL << std::endl;
	}
	RoundData roundData_MsgPrecommitFast = msgPrecommitFast.roundData;
	Signs signs_MsgPrecommitFast = msgPrecommitFast.signs;
	View proposeView_MsgPrecommitFast = roundData_MsgPrecommitFast.getProposeView();
	Phase phase_MsgPrecommitFast = roundData_MsgPrecommitFast.getPhase();
	Justification justification_MsgPrecommit = Justification(roundData_MsgPrecommitFast, signs_MsgPrecommitFast);

	if (proposeView_MsgPrecommitFast >= this->view && phase_MsgPrecommitFast == PHASE_PRECOMMIT_FAST)
	{
		if (proposeView_MsgPrecommitFast == this->view)
		{
			if (this->path == FAST_PATH)
			{
				if (this->amCurrentLeader())
				{
					if (!this->amTrustfailReplicaIds())
					{
						if (this->log.storeMsgPrecommitFast(msgPrecommitFast) >= this->trustedQuorumSize && this->log.checkMsgPrecommitFast(this->view, this->trustedQuorumSize) && !this->isHandleMsgPrecommitFast)
						{
							this->initiateMsgPrecommitFast(roundData_MsgPrecommitFast);
							this->isHandleMsgPrecommitFast = true;
						}
						else if (this->log.storeMsgPrecommitFast(msgPrecommitFast) == NUM_COMMITTEE_MEMBERS)
						{
							this->initiateMsgPrecommitFast(roundData_MsgPrecommitFast);
						}
					}
					else
					{
						if (this->log.storeMsgPrecommitFast(msgPrecommitFast) == this->trustedQuorumSize)
						{
							this->initiateMsgPrecommitFast(roundData_MsgPrecommitFast);
						}
					}
				}
				else
				{
					bool isFail_MsgPrecommitFast = msgPrecommitFast.isFail;
					Validation validation_MsgPrecommitFast = Validation();
					if (signs_MsgPrecommitFast.getSize() == this->trustedQuorumSize && !isFail_MsgPrecommitFast)
					{
						validation_MsgPrecommitFast = this->checkBlock(justification_MsgPrecommit, isFail_MsgPrecommitFast);
						if (DEBUG_HELP)
						{
							std::cout << COLOUR_BLUE << this->printReplicaId() << "Checking MsgPrecommit in fast path: " << validation_MsgPrecommitFast.toPrint() << COLOUR_NORMAL << std::endl;
						}

						if (validation_MsgPrecommitFast.isAccepted())
						{
							Hash verifyHash_Checkpoint = this->blocks[this->view].getPreviousHash();
							View verifyView_Checkpoint = this->view - 1;
							Validations validations_Checkpoint = this->validations[this->view];
							Signs signs_Checkpoint = signs_MsgPrecommitFast;
							this->updateCheckpoint(verifyHash_Checkpoint, verifyView_Checkpoint, validations_Checkpoint, signs_Checkpoint);
						}
					}
					else
					{
						validation_MsgPrecommitFast = this->checkBlock(justification_MsgPrecommit, isFail_MsgPrecommitFast);
					}
					this->executeBlockFast(roundData_MsgPrecommitFast, validation_MsgPrecommitFast);
				}
			}
			else
			{
				if (DEBUG_BASIC)
				{
					std::cout << COLOUR_RED << this->printReplicaId() << "Fail to handle MsgPrecommit in fast path" << COLOUR_NORMAL << std::endl;
				}
			}
		}
		else
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing MsgPrecommit in fast path: " << msgPrecommitFast.toPrint() << COLOUR_NORMAL << std::endl;
			}
			this->log.storeMsgPrecommitFast(msgPrecommitFast);
		}
	}

	auto end = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	statistics.addHandleTimes(this->view, time);
}

void ResiBFT::handleMsgValidationFast(MsgValidationFast msgValidationFast)
{
	auto start = std::chrono::steady_clock::now();

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Handling MsgValidation in fast path: " << msgValidationFast.toPrint() << COLOUR_NORMAL << std::endl;
	}
	Block block = msgValidationFast.block;
	Validations validations_MsgValidationFast = msgValidationFast.validations;
	RoundData roundData_MsgValidationFast = msgValidationFast.roundData;
	Signs signs_MsgValidationFast = msgValidationFast.signs;
	View proposeView_MsgValidationFast = roundData_MsgValidationFast.getProposeView();
	Phase phase_MsgValidationFast = roundData_MsgValidationFast.getPhase();
	Justification justification_MsgValidationFast = Justification(roundData_MsgValidationFast, signs_MsgValidationFast);

	if (proposeView_MsgValidationFast >= this->view && phase_MsgValidationFast == PHASE_PRECOMMIT_FAST)
	{
		if (proposeView_MsgValidationFast == this->view)
		{
			if (this->path == FAST_PATH)
			{
				if (signs_MsgValidationFast.getSize() == this->trustedQuorumSize && this->verifyJustification(justification_MsgValidationFast))
				{
					this->blocks[this->view] = block;
					this->validations[this->view] = validations_MsgValidationFast;
					bool isFail_MsgValidationFast = validations_MsgValidationFast.isAccepted();

					Validation validation_MsgValidationFast = this->checkBlock(justification_MsgValidationFast, isFail_MsgValidationFast);
					if (DEBUG_HELP)
					{
						std::cout << COLOUR_BLUE << this->printReplicaId() << "Checking MsgValidation in fast path: " << validation_MsgValidationFast.toPrint() << COLOUR_NORMAL << std::endl;
					}

					if (validation_MsgValidationFast.isAccepted())
					{
						Hash verifyHash_Checkpoint = this->blocks[this->view].getPreviousHash();
						View verifyView_Checkpoint = this->view - 1;
						Validations validations_Checkpoint = this->validations[this->view];
						Signs signs_Checkpoint = signs_MsgValidationFast;
						this->updateCheckpoint(verifyHash_Checkpoint, verifyView_Checkpoint, validations_Checkpoint, signs_Checkpoint);
					}

					this->executeBlockFast(roundData_MsgValidationFast, validation_MsgValidationFast);
				}
			}
			else
			{
				if (DEBUG_BASIC)
				{
					std::cout << COLOUR_RED << this->printReplicaId() << "Fail to handle MsgValidation in fast path" << COLOUR_NORMAL << std::endl;
				}
			}
		}
		else
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing MsgValidation in fast path: " << msgValidationFast.toPrint() << COLOUR_NORMAL << std::endl;
			}
			this->log.storeMsgValidationFast(msgValidationFast);
		}
	}

	auto end = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	statistics.addHandleTimes(this->view, time);
}

void ResiBFT::handleMsgLdrprepareFast2Common(MsgLdrprepareFast2Common MsgLdrprepareFast2Common)
{
	auto start = std::chrono::steady_clock::now();

	if (DEBUG_BASIC)
	{
		std::cout << COLOUR_RED << this->printReplicaId() << "Change from fast path to common path" << COLOUR_NORMAL << std::endl;
	}
	this->fast2common();
	statistics.addFast2common(this->view);

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Handling MsgLdrprepare for fast path to common path: " << MsgLdrprepareFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
	}
	ProposalCommon proposalCommon_MsgLdrprepareFast2Common = MsgLdrprepareFast2Common.proposalCommon;
	Committee committee_MsgLdrprepareFast2Common = MsgLdrprepareFast2Common.committee;
	Signs signs_MsgLdrprepareFast2Common = MsgLdrprepareFast2Common.signs;
	Justification justification_MsgNewviewFast = proposalCommon_MsgLdrprepareFast2Common.getJustification();
	RoundData roundData_MsgNewviewFast = justification_MsgNewviewFast.getRoundData();
	View proposeView_MsgNewviewFast = roundData_MsgNewviewFast.getProposeView();
	Hash justifyHash_MsgNewviewFast = roundData_MsgNewviewFast.getJustifyHash();
	Block block = proposalCommon_MsgLdrprepareFast2Common.getBlock();

	// Verify the [signs_MsgLdrprepareFast2Common] in [MsgLdrprepareFast2Common]
	if (this->verifyProposalCommon(proposalCommon_MsgLdrprepareFast2Common, signs_MsgLdrprepareFast2Common) && block.extends(justifyHash_MsgNewviewFast) && proposeView_MsgNewviewFast >= this->view)
	{
		if (proposeView_MsgNewviewFast == this->view)
		{
			if (this->path == FAST_PATH)
			{
				this->respondMsgLdrprepareFast2Common(justification_MsgNewviewFast, committee_MsgLdrprepareFast2Common, block);
			}
			else
			{
				if (DEBUG_BASIC)
				{
					std::cout << COLOUR_RED << this->printReplicaId() << "Fail to handle MsgLdrprepare for fast path to common path" << COLOUR_NORMAL << std::endl;
				}
			}
		}
		else
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing MsgLdrprepare for fast path to common path: " << MsgLdrprepareFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
			}
			this->log.storeMsgLdrprepareFast2Common(MsgLdrprepareFast2Common);
		}
	}

	auto end = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	statistics.addHandleTimes(this->view, time);
}

void ResiBFT::handleMsgPrepareFast2Common(MsgPrepareFast2Common msgPrepareFast2Common)
{
	auto start = std::chrono::steady_clock::now();

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Handling MsgPrepare for fast path to common path: " << msgPrepareFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
	}
	RoundData roundData_MsgPrepareFast2Common = msgPrepareFast2Common.roundData;
	Signs signs_MsgPrepare = msgPrepareFast2Common.signs;
	View proposeView_MsgPrepareFast2Common = roundData_MsgPrepareFast2Common.getProposeView();
	Phase phase_MsgPrepareFast2Common = roundData_MsgPrepareFast2Common.getPhase();
	Justification justification_MsgPrepareFast2Common = Justification(roundData_MsgPrepareFast2Common, signs_MsgPrepare);

	if (proposeView_MsgPrepareFast2Common >= this->view && phase_MsgPrepareFast2Common == PHASE_PREPARE_COMMON)
	{
		if (proposeView_MsgPrepareFast2Common == this->view)
		{
			if (this->path == FAST_PATH)
			{
				if (this->amCurrentLeader())
				{
					if (this->log.storeMsgPrepareFast2Common(msgPrepareFast2Common) == this->generalQuorumSize)
					{
						this->initiateMsgPrepareFast2Common(roundData_MsgPrepareFast2Common);
					}
				}
				else
				{
					if (signs_MsgPrepare.getSize() == this->generalQuorumSize)
					{
						this->respondMsgPrepareFast2Common(justification_MsgPrepareFast2Common);
					}
				}
			}
			else
			{
				if (DEBUG_BASIC)
				{
					std::cout << COLOUR_RED << this->printReplicaId() << "Fail to handle MsgPrepare for fast path to common path" << COLOUR_NORMAL << std::endl;
				}
			}
		}
		else
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing MsgPrepare for fast path to common path: " << msgPrepareFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
			}
			this->log.storeMsgPrepareFast2Common(msgPrepareFast2Common);
		}
	}

	auto end = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	statistics.addHandleTimes(this->view, time);
}

void ResiBFT::handleMsgPrecommitFast2Common(MsgPrecommitFast2Common msgPrecommitFast2Common)
{
	auto start = std::chrono::steady_clock::now();

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Handling MsgPrecommit for fast path to common path: " << msgPrecommitFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
	}
	RoundData roundData_MsgPrecommitFast2Common = msgPrecommitFast2Common.roundData;
	Signs signs_MsgPrecommitFast2Common = msgPrecommitFast2Common.signs;
	View proposeView_MsgPrecommitFast2Common = roundData_MsgPrecommitFast2Common.getProposeView();
	Phase phase_MsgPrecommitFast2Common = roundData_MsgPrecommitFast2Common.getPhase();
	Justification justification_MsgPrecommitFast2Common = Justification(roundData_MsgPrecommitFast2Common, signs_MsgPrecommitFast2Common);

	if (proposeView_MsgPrecommitFast2Common >= this->view && phase_MsgPrecommitFast2Common == PHASE_PRECOMMIT_COMMON)
	{
		if (proposeView_MsgPrecommitFast2Common == this->view)
		{
			if (this->path == FAST_PATH)
			{
				if (this->amCurrentLeader())
				{
					if (this->log.storeMsgPrecommitFast2Common(msgPrecommitFast2Common) == this->generalQuorumSize)
					{
						this->initiateMsgPrecommitFast2Common(roundData_MsgPrecommitFast2Common);
					}
				}
				else
				{
					if (signs_MsgPrecommitFast2Common.getSize() == this->generalQuorumSize)
					{
						this->respondMsgPrecommitFast2Common(justification_MsgPrecommitFast2Common);
					}
				}
			}
			else
			{
				if (DEBUG_BASIC)
				{
					std::cout << COLOUR_RED << this->printReplicaId() << "Fail to handle MsgPrecommit for fast path to common path" << COLOUR_NORMAL << std::endl;
				}
			}
		}
		else
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing MsgPrecommit for fast path to common path: " << msgPrecommitFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
			}
			this->log.storeMsgPrecommitFast2Common(msgPrecommitFast2Common);
		}
	}

	auto end = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	statistics.addHandleTimes(this->view, time);
}

void ResiBFT::handleMsgCommitFast2Common(MsgCommitFast2Common msgCommitFast2Common)
{
	auto start = std::chrono::steady_clock::now();

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Handling MsgCommit for fast path to common path: " << msgCommitFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
	}
	RoundData roundData_MsgCommitFast2Common = msgCommitFast2Common.roundData;
	Signs signs_MsgCommitFast2Common = msgCommitFast2Common.signs;
	View proposeView_MsgCommitFast2Common = roundData_MsgCommitFast2Common.getProposeView();
	Phase phase_MsgCommitFast2Common = roundData_MsgCommitFast2Common.getPhase();
	Justification justification_MsgCommitFast2Common = Justification(roundData_MsgCommitFast2Common, signs_MsgCommitFast2Common);

	if (proposeView_MsgCommitFast2Common >= this->view && phase_MsgCommitFast2Common == PHASE_COMMIT_COMMON)
	{
		if (proposeView_MsgCommitFast2Common == this->view)
		{
			if (this->path == FAST_PATH)
			{
				if (this->amCurrentLeader())
				{
					if (this->log.storeMsgCommitFast2Common(msgCommitFast2Common) == this->generalQuorumSize)
					{
						this->initiateMsgCommitFast2Common(roundData_MsgCommitFast2Common);
					}
				}
				else
				{
					if (signs_MsgCommitFast2Common.getSize() == this->generalQuorumSize)
					{
						this->executeBlockCommon(roundData_MsgCommitFast2Common);
					}
				}
			}
			else
			{
				if (DEBUG_BASIC)
				{
					std::cout << COLOUR_RED << this->printReplicaId() << "Fail to handle MsgCommit for fast path to common path" << COLOUR_NORMAL << std::endl;
				}
			}
		}
		else
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing MsgCommit for fast path to common path: " << msgCommitFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
			}
			this->log.storeMsgCommitFast2Common(msgCommitFast2Common);
		}
	}

	auto end = std::chrono::steady_clock::now();
	double time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	statistics.addHandleTimes(this->view, time);
}

// Initiate messages
void ResiBFT::initiateMsgNewviewCommon()
{
	Committee committee_MsgLdrprepareCommon = Committee();

	// Select [committee_MsgLdrprepareCommon]
	if (this->amTrustedReplicaIds() && this->path == COMMON_PATH)
	{
		committee_MsgLdrprepareCommon = this->selectRandomCommittee(this->trustedRecords);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Selected committee: " << committee_MsgLdrprepareCommon.toPrint() << COLOUR_NORMAL << std::endl;
		}
	}
	if (committee_MsgLdrprepareCommon.isSet())
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing the committee in view " << this->view << ": " << committee_MsgLdrprepareCommon.toPrint() << COLOUR_NORMAL << std::endl;
		}
		this->committee = committee_MsgLdrprepareCommon;
	}

	// Create [block] extends the highest prepared block
	Justification justification_MsgNewviewCommon = this->log.findHighestMsgNewviewCommon(this->view);
	RoundData roundData_MsgNewviewCommon = justification_MsgNewviewCommon.getRoundData();
	Hash justifyHash_MsgNewviewCommon = roundData_MsgNewviewCommon.getJustifyHash();
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Highest Newview for view " << this->view << ": " << justification_MsgNewviewCommon.toPrint() << COLOUR_NORMAL << std::endl;
	}
	Block block = this->createNewBlock(justifyHash_MsgNewviewCommon);

	// Create [justification_MsgPrepareCommon] for that [block]
	Justification justification_MsgPrepareCommon = this->respondProposalCommon(block.hash(), justification_MsgNewviewCommon);
	if (justification_MsgPrepareCommon.isSet())
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing block for view " << this->view << ": " << block.toPrint() << COLOUR_NORMAL << std::endl;
		}
		this->blocks[this->view] = block;

		// Create [msgLdrprepareCommon] out of [block]
		ProposalCommon proposal_MsgLdrprepareCommon = ProposalCommon(justification_MsgNewviewCommon, block);
		Signs signs_MsgLdrprepareCommon = this->initializeMsgLdrprepareCommon(proposal_MsgLdrprepareCommon);
		MsgLdrprepareCommon msgLdrprepareCommon = MsgLdrprepareCommon(proposal_MsgLdrprepareCommon, committee_MsgLdrprepareCommon, signs_MsgLdrprepareCommon);

		// Send [msgLdrprepareCommon] to replicas
		Peers recipients = this->removeFromPeers(this->replicaId);
		this->sendMsgLdrprepareCommon(msgLdrprepareCommon, recipients);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgLdrprepare to replicas in common path: " << msgLdrprepareCommon.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Create [msgPrepareCommon]
		RoundData roundData_MsgPrepareCommon = justification_MsgPrepareCommon.getRoundData();
		Signs signs_MsgPrepareCommon = justification_MsgPrepareCommon.getSigns();
		MsgPrepareCommon msgPrepareCommon = MsgPrepareCommon(roundData_MsgPrepareCommon, signs_MsgPrepareCommon);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Hold on MsgPrepare to its own in common path: " << msgPrepareCommon.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Store own [msgPrepareCommon] in the log
		if (this->log.storeMsgPrepareCommon(msgPrepareCommon) >= this->generalQuorumSize)
		{
			this->initiateMsgPrepareCommon(justification_MsgPrepareCommon.getRoundData());
		}
	}
	else
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Bad justification of MsgPrepare in common path: " << justification_MsgPrepareCommon.toPrint() << COLOUR_NORMAL << std::endl;
		}
	}
}

void ResiBFT::initiateMsgPrepareCommon(RoundData roundData_MsgPrepareCommon)
{
	View proposeView_MsgPrepareCommon = roundData_MsgPrepareCommon.getProposeView();
	Signs signs_MsgPrepareCommon = this->log.getMsgPrepareCommon(proposeView_MsgPrepareCommon, this->generalQuorumSize);
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "MsgPrepare signatures in common path: " << signs_MsgPrepareCommon.toPrint() << COLOUR_NORMAL << std::endl;
	}

	Justification justification_MsgPrepareCommon = Justification(roundData_MsgPrepareCommon, signs_MsgPrepareCommon);
	if (signs_MsgPrepareCommon.getSize() == this->generalQuorumSize)
	{
		// Create [msgPrepareCommon]
		MsgPrepareCommon msgPrepareCommon = MsgPrepareCommon(roundData_MsgPrepareCommon, signs_MsgPrepareCommon);

		// Send [msgPrepareCommon] to replicas
		Peers recipients = this->removeFromPeers(this->replicaId);
		this->sendMsgPrepareCommon(msgPrepareCommon, recipients);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgPrepare to replicas in common path: " << msgPrepareCommon.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Create [msgPrecommitCommon]
		Justification justification_MsgPrecommitCommon = this->saveMsgPrepareCommon(justification_MsgPrepareCommon);
		RoundData roundData_MsgPrecommitCommon = justification_MsgPrecommitCommon.getRoundData();
		Signs signs_MsgPrecommitCommon = justification_MsgPrecommitCommon.getSigns();
		MsgPrecommitCommon msgPrecommitCommon = MsgPrecommitCommon(roundData_MsgPrecommitCommon, signs_MsgPrecommitCommon);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Hold on MsgPrecommit to its own in common path: " << msgPrecommitCommon.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Store own [msgPrecommitCommon] in the log
		if (this->log.storeMsgPrecommitCommon(msgPrecommitCommon) >= this->generalQuorumSize)
		{
			this->initiateMsgPrecommitCommon(justification_MsgPrecommitCommon.getRoundData());
		}
	}
}

void ResiBFT::initiateMsgPrecommitCommon(RoundData roundData_MsgPrecommitCommon)
{
	View proposeView_MsgPrecommitCommon = roundData_MsgPrecommitCommon.getProposeView();
	Signs signs_MsgPrecommitCommon = this->log.getMsgPrecommitCommon(proposeView_MsgPrecommitCommon, this->generalQuorumSize);
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "MsgPrecommit signatures: " << signs_MsgPrecommitCommon.toPrint() << COLOUR_NORMAL << std::endl;
	}

	Justification justification_MsgPrecommitCommon = Justification(roundData_MsgPrecommitCommon, signs_MsgPrecommitCommon);
	if (signs_MsgPrecommitCommon.getSize() == this->generalQuorumSize)
	{
		// Create [msgPrecommitCommon]
		MsgPrecommitCommon msgPrecommitCommon = MsgPrecommitCommon(roundData_MsgPrecommitCommon, signs_MsgPrecommitCommon);

		// Send [msgPrecommitCommon] to replicas
		Peers recipients = this->removeFromPeers(this->replicaId);
		this->sendMsgPrecommitCommon(msgPrecommitCommon, recipients);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgPrecommit to replicas in common path: " << msgPrecommitCommon.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Create [msgCommitCommon]
		Justification justification_MsgCommitCommon = this->lockMsgPrecommitCommon(justification_MsgPrecommitCommon);
		RoundData roundData_MsgCommitCommon = justification_MsgCommitCommon.getRoundData();
		Signs signs_MsgCommitCommon = justification_MsgCommitCommon.getSigns();
		MsgCommitCommon msgCommitCommon = MsgCommitCommon(roundData_MsgCommitCommon, signs_MsgCommitCommon);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Hold on MsgCommit to its own in common path: " << msgCommitCommon.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Store own [msgCommitCommon] in the log
		if (this->log.storeMsgCommitCommon(msgCommitCommon) >= this->generalQuorumSize)
		{
			this->initiateMsgCommitCommon(justification_MsgCommitCommon.getRoundData());
		}
	}
}

void ResiBFT::initiateMsgCommitCommon(RoundData roundData_MsgCommitCommon)
{
	View proposeView_MsgCommitCommon = roundData_MsgCommitCommon.getProposeView();
	Signs signs_MsgCommitCommon = this->log.getMsgCommitCommon(proposeView_MsgCommitCommon, this->generalQuorumSize);
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "MsgCommit signatures: " << signs_MsgCommitCommon.toPrint() << COLOUR_NORMAL << std::endl;
	}

	if (signs_MsgCommitCommon.getSize() == this->generalQuorumSize)
	{
		// Create [msgCommitCommon]
		MsgCommitCommon msgCommitCommon = MsgCommitCommon(roundData_MsgCommitCommon, signs_MsgCommitCommon);

		// Send [msgCommitCommon] to replicas
		Peers recipients = this->removeFromPeers(this->replicaId);
		this->sendMsgCommitCommon(msgCommitCommon, recipients);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgCommit to replicas in common path: " << msgCommitCommon.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Execute the block
		this->executeBlockCommon(roundData_MsgCommitCommon);
	}
}

void ResiBFT::initiateMsgNewviewFast()
{
	std::set<MsgNewviewFast> msgNewviewsFast = this->log.getMsgNewviewFast(this->view, this->generalQuorumSize);
	if (msgNewviewsFast.size() == this->generalQuorumSize)
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Checking validations in MsgNewview" << COLOUR_NORMAL << std::endl;
		}
		Validations validations_MsgLdrprepareFast = this->buildValidations(msgNewviewsFast);
		this->validations[this->view] = validations_MsgLdrprepareFast;

		// Check [validations_MsgLdrprepareFast] from the last view
		if (validations_MsgLdrprepareFast.isAccepted())
		{
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Handling MsgNewview to accumulator" << COLOUR_NORMAL << std::endl;
			}
			Accumulator accumulator_MsgLdrprepareFast = this->buildAccumulator(msgNewviewsFast);

			if (accumulator_MsgLdrprepareFast.isSet())
			{
				// Create [block] extends the highest prepared block
				Hash prepareHash_MsgLdrprepareFast = accumulator_MsgLdrprepareFast.getPrepareHash();
				Block block = this->createNewBlock(prepareHash_MsgLdrprepareFast);

				// Create [justification_MsgPrepareFast] for that [block]
				Justification justification_MsgPrepareFast = this->respondProposalFast(block.hash(), accumulator_MsgLdrprepareFast);
				if (justification_MsgPrepareFast.isSet())
				{
					if (DEBUG_HELP)
					{
						std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing block for view " << this->view << ": " << block.toPrint() << COLOUR_NORMAL << std::endl;
					}
					this->blocks[this->view] = block;

					// Create [msgLdrprepare] out of [block]
					ProposalFast proposal_MsgLdrprepareFast = ProposalFast(accumulator_MsgLdrprepareFast, block);
					Signs signs_MsgLdrprepareFast = this->initializeMsgLdrprepareFast(proposal_MsgLdrprepareFast);
					MsgLdrprepareFast msgLdrprepareFast = MsgLdrprepareFast(proposal_MsgLdrprepareFast, validations_MsgLdrprepareFast, signs_MsgLdrprepareFast);

					// Send [msgLdrprepareFast] to committee members
					Peers recipients = this->removeFromCommitteePeers(this->committee, this->replicaId);
					this->sendMsgLdrprepareFast(msgLdrprepareFast, recipients);
					if (DEBUG_HELP)
					{
						std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgLdrprepare to committee members in fast path: " << msgLdrprepareFast.toPrint() << COLOUR_NORMAL << std::endl;
					}

					// Create [msgPrepareFast]
					RoundData roundData_MsgPrepareFast = justification_MsgPrepareFast.getRoundData();
					Signs signs_MsgPrepareFast = justification_MsgPrepareFast.getSigns();
					MsgPrepareFast msgPrepareFast = MsgPrepareFast(roundData_MsgPrepareFast, signs_MsgPrepareFast);
					if (this->amTrustfailReplicaIds())
					{
						bool isFail_MsgPrepareFast = true;
						msgPrepareFast = MsgPrepareFast(isFail_MsgPrepareFast, roundData_MsgPrepareFast, signs_MsgPrepareFast);
					}

					if (DEBUG_HELP)
					{
						std::cout << COLOUR_BLUE << this->printReplicaId() << "Hold on MsgPrepare to its own in fast path: " << msgPrepareFast.toPrint() << COLOUR_NORMAL << std::endl;
					}

					// Store own [msgPrepareFast] in the log
					if (this->log.storeMsgPrepareFast(msgPrepareFast) == this->trustedQuorumSize)
					{
						this->initiateMsgPrepareFast(msgPrepareFast.roundData);
					}
				}
			}
			else
			{
				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Bad accumulator for MsgLdrprepare: " << accumulator_MsgLdrprepareFast.toPrint() << COLOUR_NORMAL << std::endl;
				}
			}
		}
		else
		{
			if (DEBUG_BASIC)
			{
				std::cout << COLOUR_RED << this->printReplicaId() << "Change from fast path to common path" << COLOUR_NORMAL << std::endl;
			}
			this->fast2common();
			statistics.addFast2common(this->view);

			// Set [committee_MsgLdrprepareFast2Common]
			bool set_Committee = true;
			Committee committee_MsgLdrprepareFast2Common = Committee(set_Committee);
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Cancel committee: " << committee_MsgLdrprepareFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
			}
			this->committee = committee_MsgLdrprepareFast2Common;

			// Create [block] extends the highest prepared block
			Justification justification_MsgNewviewFast = this->log.findHighestMsgNewviewFast(this->view);
			RoundData roundData_MsgNewviewFast = justification_MsgNewviewFast.getRoundData();
			Hash justifyHash_MsgNewviewFast = roundData_MsgNewviewFast.getJustifyHash();
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Highest Newview for view " << this->view << ": " << justification_MsgNewviewFast.toPrint() << COLOUR_NORMAL << std::endl;
			}
			Block block = this->createNewBlock(justifyHash_MsgNewviewFast);

			// Create [justification_MsgPrepareFast2Common] for that [block]
			Justification justification_MsgPrepareFast2Common = this->respondProposalFast2Common(block.hash(), justification_MsgNewviewFast);
			if (justification_MsgPrepareFast2Common.isSet())
			{
				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing block for view " << this->view << ": " << block.toPrint() << COLOUR_NORMAL << std::endl;
				}
				this->blocks[this->view] = block;

				// Create [msgLdrprepareFast2Common] out of [block]
				ProposalCommon proposal_MsgLdrprepareFast2Common = ProposalCommon(justification_MsgNewviewFast, block);
				Signs signs_MsgLdrprepareFast2Common = this->initializeMsgLdrprepareFast2Common(proposal_MsgLdrprepareFast2Common);
				MsgLdrprepareFast2Common msgLdrprepareFast2Common = MsgLdrprepareFast2Common(proposal_MsgLdrprepareFast2Common, committee_MsgLdrprepareFast2Common, signs_MsgLdrprepareFast2Common);

				// Send [msgLdrprepareFast2Common] to replicas
				Peers recipients = this->removeFromPeers(this->replicaId);
				this->sendMsgLdrprepareFast2Common(msgLdrprepareFast2Common, recipients);
				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgLdrprepare to replicas for fast path to common path: " << msgLdrprepareFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
				}

				// Create [msgPrepareFast2Common]
				RoundData roundData_MsgPrepareFast2Common = justification_MsgPrepareFast2Common.getRoundData();
				Signs signs_MsgPrepareFast2Common = justification_MsgPrepareFast2Common.getSigns();
				MsgPrepareFast2Common msgPrepareFast2Common = MsgPrepareFast2Common(roundData_MsgPrepareFast2Common, signs_MsgPrepareFast2Common);
				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Hold on MsgPrepare to its own for fast path to common path: " << msgPrepareFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
				}

				// Store own [msgPrepareFast2Common] in the log
				if (this->log.storeMsgPrepareFast2Common(msgPrepareFast2Common) >= this->generalQuorumSize)
				{
					this->initiateMsgPrepareFast2Common(justification_MsgPrepareFast2Common.getRoundData());
				}
			}
			else
			{
				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Bad justification of MsgPrepare for fast path to common path: " << justification_MsgPrepareFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
				}
			}
		}
	}
}

void ResiBFT::initiateMsgPrepareFast(RoundData roundData_MsgPrepareFast)
{
	View proposeView_MsgPrepareFast = roundData_MsgPrepareFast.getProposeView();
	Signs signs_MsgPrepareFast = this->log.getMsgPrepareFast(proposeView_MsgPrepareFast, this->trustedQuorumSize);
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "MsgPrepare signatures in fast path: " << signs_MsgPrepareFast.toPrint() << COLOUR_NORMAL << std::endl;
	}

	Justification justification_MsgPrepareFast = Justification(roundData_MsgPrepareFast, signs_MsgPrepareFast);
	if (signs_MsgPrepareFast.getSize() == this->trustedQuorumSize)
	{
		// Create [msgPrepareFast]
		MsgPrepareFast msgPrepareFast = MsgPrepareFast(roundData_MsgPrepareFast, signs_MsgPrepareFast);

		// Send [msgPrepareFast] to committee members
		Peers recipients = this->removeFromCommitteePeers(this->committee, this->replicaId);
		this->sendMsgPrepareFast(msgPrepareFast, recipients);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgPrepare to replicas in fast path: " << msgPrepareFast.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Create [msgPrecommitFast]
		bool isFail_MsgPrepareFast = false;
		Justification justification_MsgPrecommitFast = this->saveMsgPrepareFast(justification_MsgPrepareFast, isFail_MsgPrepareFast);
		RoundData roundData_MsgPrecommitFast = justification_MsgPrecommitFast.getRoundData();
		Signs signs_MsgPrecommitFast = justification_MsgPrecommitFast.getSigns();
		MsgPrecommitFast msgPrecommitFast = MsgPrecommitFast(roundData_MsgPrecommitFast, signs_MsgPrecommitFast);
		if (this->amTrustfailReplicaIds())
		{
			bool isFail_MsgPrecommitFast = true;
			msgPrecommitFast = MsgPrecommitFast(isFail_MsgPrecommitFast, roundData_MsgPrecommitFast, signs_MsgPrecommitFast);
		}

		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Hold on MsgPrecommit to its own in fast path: " << msgPrecommitFast.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Store own [msgPrecommitFast] in the log
		if (this->log.storeMsgPrecommitFast(msgPrecommitFast) >= this->trustedQuorumSize)
		{
			this->initiateMsgPrecommitFast(justification_MsgPrecommitFast.getRoundData());
		}
	}
	else
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Bad MsgPrepare signatures: " << signs_MsgPrepareFast.toPrint() << COLOUR_NORMAL << std::endl;
		}

		Signs signs_MsgPrepareFastAll = this->log.getMsgPrepareFastAll(proposeView_MsgPrepareFast, this->trustedQuorumSize);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "MsgPrepare signatures in fast path: " << signs_MsgPrepareFastAll.toPrint() << COLOUR_NORMAL << std::endl;
		}

		Justification justification_MsgPrepareFast = Justification(roundData_MsgPrepareFast, signs_MsgPrepareFastAll);
		if (signs_MsgPrepareFastAll.getSize() == this->trustedQuorumSize)
		{
			// Create [msgPrepareFast]
			bool isFail_MsgPrepareFast = true;
			MsgPrepareFast msgPrepareFast = MsgPrepareFast(isFail_MsgPrepareFast, roundData_MsgPrepareFast, signs_MsgPrepareFastAll);

			// Send [msgPrepareFast] to committee members
			Peers recipients = this->removeFromCommitteePeers(this->committee, this->replicaId);
			this->sendMsgPrepareFast(msgPrepareFast, recipients);
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent the failed MsgPrepare to replicas in fast path: " << msgPrepareFast.toPrint() << COLOUR_NORMAL << std::endl;
			}

			// Create [msgPrecommitFast]
			Justification justification_MsgPrecommitFast = this->saveMsgPrepareFast(justification_MsgPrepareFast, isFail_MsgPrepareFast);
			RoundData roundData_MsgPrecommitFast = justification_MsgPrecommitFast.getRoundData();
			Signs signs_MsgPrecommitFast = justification_MsgPrecommitFast.getSigns();
			bool isFail_MsgPrecommitFast = true;
			MsgPrecommitFast msgPrecommitFast = MsgPrecommitFast(isFail_MsgPrecommitFast, roundData_MsgPrecommitFast, signs_MsgPrecommitFast);
			if (DEBUG_HELP)
			{
				std::cout << COLOUR_BLUE << this->printReplicaId() << "Hold on the failed MsgPrecommit to its own in fast path: " << msgPrecommitFast.toPrint() << COLOUR_NORMAL << std::endl;
			}

			// Store own [msgPrecommitFast] in the log
			if (this->log.storeMsgPrecommitFast(msgPrecommitFast) >= this->trustedQuorumSize)
			{
				this->initiateMsgPrecommitFast(justification_MsgPrecommitFast.getRoundData());
			}
		}
	}
}

void ResiBFT::initiateMsgPrecommitFast(RoundData roundData_MsgPrecommitFast)
{
	View proposeView_MsgPrecommitFast = roundData_MsgPrecommitFast.getProposeView();
	Signs signs_MsgPrecommitFast = this->log.getMsgPrecommitFast(proposeView_MsgPrecommitFast, this->trustedQuorumSize);
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "MsgPrecommit signatures in fast path: " << signs_MsgPrecommitFast.toPrint() << COLOUR_NORMAL << std::endl;
	}

	if (signs_MsgPrecommitFast.getSize() == this->trustedQuorumSize)
	{
		// Create [msgPrecommitFast]
		MsgPrecommitFast msgPrecommitFast = MsgPrecommitFast(roundData_MsgPrecommitFast, signs_MsgPrecommitFast);

		// Send [msgPrecommitFast] to committee members
		Peers recipients_msgPrecommitFast = this->removeFromCommitteePeers(this->committee, this->replicaId);
		this->sendMsgPrecommitFast(msgPrecommitFast, recipients_msgPrecommitFast);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgPrecommit to replicas in fast path: " << msgPrecommitFast.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Create [msgValidationFast]
		Block block = this->blocks[this->view];
		Validations validations_MsgValidationFast = this->validations[this->view];
		RoundData roundData_MsgValidationFast = roundData_MsgPrecommitFast;
		Signs signs_MsgValidationFast = signs_MsgPrecommitFast;
		MsgValidationFast msgValidationFast = MsgValidationFast(block, validations_MsgValidationFast, roundData_MsgValidationFast, signs_MsgValidationFast);

		// Send [msgValidationFast] to non-committee members
		Peers recipients_msgValidationFast = this->keepFromCommitteePeers(this->committee, this->replicaId);
		this->sendMsgValidationFast(msgValidationFast, recipients_msgValidationFast);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent msgValidation to replicas in fast path: " << msgValidationFast.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Update [validation_MsgPrecommitFast]
		Justification justification_MsgPrecommitFast = Justification(roundData_MsgPrecommitFast, signs_MsgPrecommitFast);
		bool isFail_MsgPrecommitFast = false;

		Validation validation_MsgPrecommitFast = this->checkBlock(justification_MsgPrecommitFast, isFail_MsgPrecommitFast);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Checking MsgValidation in fast path: " << validation_MsgPrecommitFast.toPrint() << COLOUR_NORMAL << std::endl;
		}

		if (validation_MsgPrecommitFast.isAccepted())
		{
			Hash verifyHash_Checkpoint = this->blocks[this->view].getPreviousHash();
			View verifyView_Checkpoint = this->view - 1;
			Validations validations_Checkpoint = this->validations[this->view];
			Signs signs_Checkpoint = signs_MsgPrecommitFast;
			this->updateCheckpoint(verifyHash_Checkpoint, verifyView_Checkpoint, validations_Checkpoint, signs_Checkpoint);
		}

		// Execute the block
		this->executeBlockFast(roundData_MsgPrecommitFast, validation_MsgPrecommitFast);
	}
	else
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Bad MsgPrecommit signatures: " << signs_MsgPrecommitFast.toPrint() << COLOUR_NORMAL << std::endl;
		}

		Signs signs_MsgPrecommitFastAll = this->log.getMsgPrecommitFastAll(proposeView_MsgPrecommitFast, this->trustedQuorumSize);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "MsgPrecommit signatures in fast path: " << signs_MsgPrecommitFastAll.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Create [msgPrecommitFast]
		bool isFail_MsgPrecommitFast = true;
		MsgPrecommitFast msgPrecommitFast = MsgPrecommitFast(isFail_MsgPrecommitFast, roundData_MsgPrecommitFast, signs_MsgPrecommitFastAll);

		// Send [msgPrecommitFast] to committee members
		Peers recipients_msgPrecommitFast = this->removeFromCommitteePeers(this->committee, this->replicaId);
		this->sendMsgPrecommitFast(msgPrecommitFast, recipients_msgPrecommitFast);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent the failed MsgPrecommit to replicas in fast path: " << msgPrecommitFast.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Create [msgValidationFast]
		Block block = this->blocks[this->view];
		Validations validations_MsgValidationFast = this->validations[this->view];
		RoundData roundData_MsgValidationFast = roundData_MsgPrecommitFast;
		Signs signs_MsgValidationFast = signs_MsgPrecommitFastAll;
		MsgValidationFast msgValidationFast = MsgValidationFast(block, validations_MsgValidationFast, roundData_MsgValidationFast, signs_MsgValidationFast);

		// Send [msgValidationFast] to non-committee members
		Peers recipients_msgValidationFast = this->keepFromCommitteePeers(this->committee, this->replicaId);
		this->sendMsgValidationFast(msgValidationFast, recipients_msgValidationFast);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent msgValidation to replicas in fast path: " << msgValidationFast.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Update [validation_MsgPrecommitFast]
		Justification justification_MsgPrecommitFast = Justification(roundData_MsgPrecommitFast, signs_MsgPrecommitFastAll);

		Validation validation_MsgPrecommitFast = this->checkBlock(justification_MsgPrecommitFast, isFail_MsgPrecommitFast);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Checking MsgValidation in fast path: " << validation_MsgPrecommitFast.toPrint() << COLOUR_NORMAL << std::endl;
		}

		if (validation_MsgPrecommitFast.isAccepted())
		{
			Hash verifyHash_Checkpoint = this->blocks[this->view].getPreviousHash();
			View verifyView_Checkpoint = this->view - 1;
			Validations validations_Checkpoint = this->validations[this->view];
			Signs signs_Checkpoint = signs_MsgPrecommitFast;
			this->updateCheckpoint(verifyHash_Checkpoint, verifyView_Checkpoint, validations_Checkpoint, signs_Checkpoint);
		}

		// Execute the block
		this->executeBlockFast(roundData_MsgPrecommitFast, validation_MsgPrecommitFast);
	}
}

void ResiBFT::initiateMsgPrepareFast2Common(RoundData roundData_MsgPrepareFast2Common)
{
	View proposeView_MsgPrepareFast2Common = roundData_MsgPrepareFast2Common.getProposeView();
	Signs signs_MsgPrepareFast2Common = this->log.getMsgPrepareFast2Common(proposeView_MsgPrepareFast2Common, this->generalQuorumSize);
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "MsgPrepare signatures for fast path to common path: " << signs_MsgPrepareFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
	}

	Justification justification_MsgPrepareFast2Common = Justification(roundData_MsgPrepareFast2Common, signs_MsgPrepareFast2Common);
	if (signs_MsgPrepareFast2Common.getSize() == this->generalQuorumSize)
	{
		// Create [msgPrepareFast2Common]
		MsgPrepareFast2Common msgPrepareFast2Common = MsgPrepareFast2Common(roundData_MsgPrepareFast2Common, signs_MsgPrepareFast2Common);

		// Send [msgPrepareFast2Common] to replicas
		Peers recipients = this->removeFromPeers(this->replicaId);
		this->sendMsgPrepareFast2Common(msgPrepareFast2Common, recipients);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgPrepare to replicas for fast path to common path: " << msgPrepareFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Create [msgPrecommitFast2Common]
		Justification justification_MsgPrecommitFast2Common = this->saveMsgPrepareFast2Common(justification_MsgPrepareFast2Common);
		RoundData roundData_MsgPrecommitFast2Common = justification_MsgPrecommitFast2Common.getRoundData();
		Signs signs_MsgPrecommitFast2Common = justification_MsgPrecommitFast2Common.getSigns();
		MsgPrecommitFast2Common msgPrecommitFast2Common = MsgPrecommitFast2Common(roundData_MsgPrecommitFast2Common, signs_MsgPrecommitFast2Common);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Hold on MsgPrecommit to its own for fast path to common path: " << msgPrecommitFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Store own [msgPrecommitFast2Common] in the log
		if (this->log.storeMsgPrecommitFast2Common(msgPrecommitFast2Common) >= this->generalQuorumSize)
		{
			this->initiateMsgPrecommitFast2Common(justification_MsgPrecommitFast2Common.getRoundData());
		}
	}
}

void ResiBFT::initiateMsgPrecommitFast2Common(RoundData roundData_MsgPrecommitFast2Common)
{
	View proposeView_MsgPrecommitFast2Common = roundData_MsgPrecommitFast2Common.getProposeView();
	Signs signs_MsgPrecommitFast2Common = this->log.getMsgPrecommitFast2Common(proposeView_MsgPrecommitFast2Common, this->generalQuorumSize);
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "MsgPrecommit signatures: " << signs_MsgPrecommitFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
	}

	Justification justification_MsgPrecommitFast2Common = Justification(roundData_MsgPrecommitFast2Common, signs_MsgPrecommitFast2Common);
	if (signs_MsgPrecommitFast2Common.getSize() == this->generalQuorumSize)
	{
		// Create [msgPrecommitFast2Common]
		MsgPrecommitFast2Common msgPrecommitFast2Common = MsgPrecommitFast2Common(roundData_MsgPrecommitFast2Common, signs_MsgPrecommitFast2Common);

		// Send [msgPrecommitFast2Common] to replicas
		Peers recipients = this->removeFromPeers(this->replicaId);
		this->sendMsgPrecommitFast2Common(msgPrecommitFast2Common, recipients);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgPrecommit to replicas for fast path to common path: " << msgPrecommitFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Create [msgCommitFast2Common]
		Justification justification_MsgCommitFast2Common = this->lockMsgPrecommitFast2Common(justification_MsgPrecommitFast2Common);
		RoundData roundData_MsgCommitFast2Common = justification_MsgCommitFast2Common.getRoundData();
		Signs signs_MsgCommitFast2Common = justification_MsgCommitFast2Common.getSigns();
		MsgCommitFast2Common msgCommitFast2Common = MsgCommitFast2Common(roundData_MsgCommitFast2Common, signs_MsgCommitFast2Common);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Hold on MsgCommit to its own for fast path to common path: " << msgCommitFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Store own [msgCommitFast2Common] in the log
		if (this->log.storeMsgCommitFast2Common(msgCommitFast2Common) >= this->generalQuorumSize)
		{
			this->initiateMsgCommitFast2Common(justification_MsgCommitFast2Common.getRoundData());
		}
	}
}

void ResiBFT::initiateMsgCommitFast2Common(RoundData roundData_MsgCommitFast2Common)
{
	View proposeView_MsgCommitFast2Common = roundData_MsgCommitFast2Common.getProposeView();
	Signs signs_MsgCommitFast2Common = this->log.getMsgCommitFast2Common(proposeView_MsgCommitFast2Common, this->generalQuorumSize);
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "MsgCommit signatures: " << signs_MsgCommitFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
	}

	if (signs_MsgCommitFast2Common.getSize() == this->generalQuorumSize)
	{
		// Create [msgCommitFast2Common]
		MsgCommitFast2Common msgCommitFast2Common = MsgCommitFast2Common(roundData_MsgCommitFast2Common, signs_MsgCommitFast2Common);

		// Send [msgCommitFast2Common] to replicas
		Peers recipients = this->removeFromPeers(this->replicaId);
		this->sendMsgCommitFast2Common(msgCommitFast2Common, recipients);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgCommit to replicas for fast path to common path: " << msgCommitFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
		}

		// Execute the block
		this->executeBlockCommon(roundData_MsgCommitFast2Common);
	}
}

// Respond messages
void ResiBFT::respondMsgLdrprepareCommon(Justification justification_MsgNewviewCommon, Committee committee_MsgLdrprepareCommon, Block block)
{
	// Update own [committee_MsgLdrprepareCommon]
	if (committee_MsgLdrprepareCommon.isSet())
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing the committee in view " << this->view << ": " << committee_MsgLdrprepareCommon.toPrint() << COLOUR_NORMAL << std::endl;
		}
		this->committee = committee_MsgLdrprepareCommon;
	}

	// Create own [justification_MsgPrepareCommon] for that [block]
	Justification justification_MsgPrepareCommon = this->respondProposalCommon(block.hash(), justification_MsgNewviewCommon);
	if (justification_MsgPrepareCommon.isSet())
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing block for view " << this->view << ": " << block.toPrint() << COLOUR_NORMAL << std::endl;
		}
		this->blocks[this->view] = block;

		// Create [msgPrepareCommon] out of [block]
		RoundData roundData_MsgPrepareCommon = justification_MsgPrepareCommon.getRoundData();
		Signs signs_MsgPrepareCommon = justification_MsgPrepareCommon.getSigns();
		MsgPrepareCommon msgPrepareCommon = MsgPrepareCommon(roundData_MsgPrepareCommon, signs_MsgPrepareCommon);

		// Send [msgPrepareCommon] to leader
		Peers recipients = this->keepFromPeers(this->getCurrentLeader());
		this->sendMsgPrepareCommon(msgPrepareCommon, recipients);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgPrepare to leader in common path: " << msgPrepareCommon.toPrint() << COLOUR_NORMAL << std::endl;
		}
	}
}

void ResiBFT::respondMsgPrepareCommon(Justification justification_MsgPrepareCommon)
{
	// Create [justification_MsgPrecommitCommon]
	Justification justification_MsgPrecommitCommon = this->saveMsgPrepareCommon(justification_MsgPrepareCommon);

	// Create [msgPrecommitCommon]
	RoundData roundData_MsgPrecommitCommon = justification_MsgPrecommitCommon.getRoundData();
	Signs signs_MsgPrecommitCommon = justification_MsgPrecommitCommon.getSigns();
	MsgPrecommitCommon msgPrecommitCommon = MsgPrecommitCommon(roundData_MsgPrecommitCommon, signs_MsgPrecommitCommon);

	// Send [msgPrecommitCommon] to leader
	Peers recipients = this->keepFromPeers(this->getCurrentLeader());
	this->sendMsgPrecommitCommon(msgPrecommitCommon, recipients);
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgPrecommit to leader in common path: " << msgPrecommitCommon.toPrint() << COLOUR_NORMAL << std::endl;
	}
}

void ResiBFT::respondMsgPrecommitCommon(Justification justification_MsgPrecommitCommon)
{
	// Create [justification_MsgCommitCommon]
	Justification justification_MsgCommitCommon = this->lockMsgPrecommitCommon(justification_MsgPrecommitCommon);

	// Create [msgCommitCommon]
	RoundData roundData_MsgCommitCommon = justification_MsgCommitCommon.getRoundData();
	Signs signs_MsgCommitCommon = justification_MsgCommitCommon.getSigns();
	MsgCommitCommon msgCommitCommon = MsgCommitCommon(roundData_MsgCommitCommon, signs_MsgCommitCommon);

	// Send [msgCommitCommon] to leader
	Peers recipients = this->keepFromPeers(this->getCurrentLeader());
	this->sendMsgCommitCommon(msgCommitCommon, recipients);
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgCommit to leader in common path: " << msgCommitCommon.toPrint() << COLOUR_NORMAL << std::endl;
	}
}

void ResiBFT::respondMsgLdrprepareFast(Accumulator accumulator_MsgLdrprepareFast, Validations validations_MsgLdrprepareFast, Block block)
{
	// Create own [justification_MsgPrepareFast] for that [block]
	Justification justification_MsgPrepareFast = this->respondProposalFast(block.hash(), accumulator_MsgLdrprepareFast);
	if (justification_MsgPrepareFast.isSet())
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing validations for view " << this->view << ": " << block.toPrint() << COLOUR_NORMAL << std::endl;
		}
		this->validations[this->view] = validations_MsgLdrprepareFast;

		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing block for view " << this->view << ": " << block.toPrint() << COLOUR_NORMAL << std::endl;
		}
		this->blocks[this->view] = block;

		// Create [msgPrepareFast] out of [block]
		RoundData roundData_MsgPrepareFast = justification_MsgPrepareFast.getRoundData();
		Signs signs_MsgPrepareFast = justification_MsgPrepareFast.getSigns();
		MsgPrepareFast msgPrepareFast = MsgPrepareFast(roundData_MsgPrepareFast, signs_MsgPrepareFast);
		if (this->amTrustfailReplicaIds() || !validations_MsgLdrprepareFast.isAccepted())
		{
			bool isFail_MsgPrepareFast = true;
			msgPrepareFast = MsgPrepareFast(isFail_MsgPrepareFast, roundData_MsgPrepareFast, signs_MsgPrepareFast);
		}

		// Send [msgPrepareFast] to leader
		Peers recipients = this->keepFromPeers(this->getCurrentLeader());
		this->sendMsgPrepareFast(msgPrepareFast, recipients);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgPrepare to leader in fast path: " << msgPrepareFast.toPrint() << COLOUR_NORMAL << std::endl;
		}
	}
}

void ResiBFT::respondMsgPrepareFast(Justification justification_MsgPrepareFast, bool isFail_MsgPrepareFast)
{
	// Create [justification_MsgPrecommitFast]
	Justification justification_MsgPrecommitFast = this->saveMsgPrepareFast(justification_MsgPrepareFast, isFail_MsgPrepareFast);

	// Create [msgPrecommitFast]
	RoundData roundData_MsgPrecommitFast = justification_MsgPrecommitFast.getRoundData();
	Signs signs_MsgPrecommitFast = justification_MsgPrecommitFast.getSigns();
	MsgPrecommitFast msgPrecommitFast = MsgPrecommitFast(roundData_MsgPrecommitFast, signs_MsgPrecommitFast);
	if (!this->amTrustfailReplicaIds() || isFail_MsgPrepareFast)
	{
		bool isFail_MsgPrecommitFast = true;
		msgPrecommitFast = MsgPrecommitFast(isFail_MsgPrecommitFast, roundData_MsgPrecommitFast, signs_MsgPrecommitFast);
	}

	// Send [msgPrecommitFast] to leader
	Peers recipients = this->keepFromPeers(this->getCurrentLeader());
	this->sendMsgPrecommitFast(msgPrecommitFast, recipients);
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgPrecommit to leader in fast path: " << msgPrecommitFast.toPrint() << COLOUR_NORMAL << std::endl;
	}
}

// Respond messages
void ResiBFT::respondMsgLdrprepareFast2Common(Justification justification_MsgNewviewFast, Committee committee_MsgLdrprepareFast2Common, Block block)
{
	// Update own [committee_MsgLdrprepareFast2Common]
	if (committee_MsgLdrprepareFast2Common.isSet())
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing the committee in view " << this->view << ": " << committee_MsgLdrprepareFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
		}
		this->committee = committee_MsgLdrprepareFast2Common;
	}

	// Create own [justification_MsgPrepareFast2Common] for that [block]
	Justification justification_MsgPrepareFast2Common = this->respondProposalFast2Common(block.hash(), justification_MsgNewviewFast);
	if (justification_MsgPrepareFast2Common.isSet())
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Storing block for view " << this->view << ": " << block.toPrint() << COLOUR_NORMAL << std::endl;
		}
		this->blocks[this->view] = block;

		// Create [msgPrepareFast2Common] out of [block]
		RoundData roundData_MsgPrepareFast2Common = justification_MsgPrepareFast2Common.getRoundData();
		Signs signs_MsgPrepareFast2Common = justification_MsgPrepareFast2Common.getSigns();
		MsgPrepareFast2Common msgPrepareFast2Common = MsgPrepareFast2Common(roundData_MsgPrepareFast2Common, signs_MsgPrepareFast2Common);

		// Send [msgPrepareFast2Common] to leader
		Peers recipients = this->keepFromPeers(this->getCurrentLeader());
		this->sendMsgPrepareFast2Common(msgPrepareFast2Common, recipients);
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgPrepare to leader for fast path to common path: " << msgPrepareFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
		}
	}
}

void ResiBFT::respondMsgPrepareFast2Common(Justification justification_MsgPrepareFast2Common)
{
	// Create [justification_MsgPrecommitFast2Common]
	Justification justification_MsgPrecommitFast2Common = this->saveMsgPrepareFast2Common(justification_MsgPrepareFast2Common);

	// Create [msgPrecommitFast2Common]
	RoundData roundData_MsgPrecommitFast2Common = justification_MsgPrecommitFast2Common.getRoundData();
	Signs signs_MsgPrecommitFast2Common = justification_MsgPrecommitFast2Common.getSigns();
	MsgPrecommitFast2Common msgPrecommitFast2Common = MsgPrecommitFast2Common(roundData_MsgPrecommitFast2Common, signs_MsgPrecommitFast2Common);

	// Send [msgPrecommitFast2Common] to leader
	Peers recipients = this->keepFromPeers(this->getCurrentLeader());
	this->sendMsgPrecommitFast2Common(msgPrecommitFast2Common, recipients);
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgPrecommit to leader for fast path to common path: " << msgPrecommitFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
	}
}

void ResiBFT::respondMsgPrecommitFast2Common(Justification justification_MsgPrecommitFast2Common)
{
	// Create [justification_MsgCommitFast2Common]
	Justification justification_MsgCommitFast2Common = this->lockMsgPrecommitFast2Common(justification_MsgPrecommitFast2Common);

	// Create [msgCommitFast2Common]
	RoundData roundData_MsgCommitFast2Common = justification_MsgCommitFast2Common.getRoundData();
	Signs signs_MsgCommitFast2Common = justification_MsgCommitFast2Common.getSigns();
	MsgCommitFast2Common msgCommitFast2Common = MsgCommitFast2Common(roundData_MsgCommitFast2Common, signs_MsgCommitFast2Common);

	// Send [msgCommitFast2Common] to leader
	Peers recipients = this->keepFromPeers(this->getCurrentLeader());
	this->sendMsgCommitFast2Common(msgCommitFast2Common, recipients);
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgCommit to leader for fast path to common path: " << msgCommitFast2Common.toPrint() << COLOUR_NORMAL << std::endl;
	}
}

// Main functions
int ResiBFT::initializeSGX()
{
	// Initializing enclave
	if (initialize_enclave(&global_eid, "enclave.token", "enclave.signed.so") < 0)
	{
		std::cout << this->printReplicaId() << "Failed to initialize enclave" << std::endl;
		return 1;
	}
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Initialized enclave" << COLOUR_NORMAL << std::endl;
	}

	// Initializing variables
	std::set<ReplicaID> replicaIds = this->nodes.getReplicaIds();
	unsigned int num = replicaIds.size();
	Pids_t others;
	others.num_nodes = num;
	unsigned int i = 0;
	for (std::set<ReplicaID>::iterator it = replicaIds.begin(); it != replicaIds.end(); it++, i++)
	{
		others.pids[i] = *it;
	}

	sgx_status_t enclave_status_t;
	sgx_status_t ecall_status_t;
	ecall_status_t = TEE_initializeVariables(global_eid, &enclave_status_t, &(this->replicaId), &others, &(this->generalQuorumSize), &(this->trustedQuorumSize));
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Enclave variables are initialized." << COLOUR_NORMAL << std::endl;
	}
	return 0;
}

void ResiBFT::getStartedCommon()
{
	if (DEBUG_BASIC)
	{
		std::cout << COLOUR_RED << this->printReplicaId() << "Starting in common path" << COLOUR_NORMAL << std::endl;
	}
	startTime = std::chrono::steady_clock::now();
	startView = std::chrono::steady_clock::now();

	// Send [msgNewviewCommon] to the leader of the current view
	Peers recipients = this->keepFromPeers(this->getCurrentLeader());

	Justification justification_MsgNewviewCommon = this->initializeMsgNewviewCommon();
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Initial justification in common path: " << justification_MsgNewviewCommon.toPrint() << COLOUR_NORMAL << std::endl;
	}
	RoundData roundData_MsgNewviewCommon = justification_MsgNewviewCommon.getRoundData();
	Signs signs_MsgNewviewCommon = justification_MsgNewviewCommon.getSigns();
	MsgNewviewCommon msgNewviewCommon = MsgNewviewCommon(roundData_MsgNewviewCommon, signs_MsgNewviewCommon);
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Starting with: " << msgNewviewCommon.toPrint() << COLOUR_NORMAL << std::endl;
	}
	if (this->amCurrentLeader())
	{
		this->handleMsgNewviewCommon(msgNewviewCommon);
	}
	else
	{
		this->sendMsgNewviewCommon(msgNewviewCommon, recipients);
	}
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Sent MsgNewview to leader[" << this->getCurrentLeader() << "] in common path" << COLOUR_NORMAL << std::endl;
	}
}

void ResiBFT::startNewViewCommon()
{
	Justification justification_MsgNewviewCommon = this->initializeMsgNewviewCommon();
	View proposeView_MsgNewviewCommon = justification_MsgNewviewCommon.getRoundData().getProposeView();
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Generating new justification in common path: " << justification_MsgNewviewCommon.toPrint() << COLOUR_NORMAL << std::endl;
	}
	while (proposeView_MsgNewviewCommon <= this->view)
	{
		justification_MsgNewviewCommon = this->initializeMsgNewviewCommon();
		proposeView_MsgNewviewCommon = justification_MsgNewviewCommon.getRoundData().getProposeView();
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Updating justification in common path: " << justification_MsgNewviewCommon.toPrint() << COLOUR_NORMAL << std::endl;
		}
	}

	// Increase the view
	this->view++;

	// Start the timer
	this->setTimer();

	RoundData roundData_MsgNewviewCommon = justification_MsgNewviewCommon.getRoundData();
	Phase phase_MsgNewviewCommon = roundData_MsgNewviewCommon.getPhase();
	Signs signs_MsgNewviewCommon = justification_MsgNewviewCommon.getSigns();
	if (proposeView_MsgNewviewCommon == this->view && phase_MsgNewviewCommon == PHASE_NEWVIEW_COMMON)
	{
		MsgNewviewCommon msgNewviewCommon = MsgNewviewCommon(roundData_MsgNewviewCommon, signs_MsgNewviewCommon);
		if (this->amCurrentLeader())
		{
			this->handleEarlierMessagesCommon();
			this->handleMsgNewviewCommon(msgNewviewCommon);
		}
		else
		{
			ReplicaID leader = this->getCurrentLeader();
			Peers recipients = this->keepFromPeers(leader);
			this->sendMsgNewviewCommon(msgNewviewCommon, recipients);
			this->handleEarlierMessagesCommon();
		}
	}
	else
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Failed to start in common path" << COLOUR_NORMAL << std::endl;
		}
	}
}

void ResiBFT::startNewViewFast(Validation validation)
{
	Justification justification_MsgNewviewFast = this->initializeMsgNewviewFast();
	View proposeView_MsgNewviewFast = justification_MsgNewviewFast.getRoundData().getProposeView();
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Generating new justification in fast path: " << justification_MsgNewviewFast.toPrint() << COLOUR_NORMAL << std::endl;
	}
	while (proposeView_MsgNewviewFast <= this->view)
	{
		justification_MsgNewviewFast = this->initializeMsgNewviewFast();
		proposeView_MsgNewviewFast = justification_MsgNewviewFast.getRoundData().getProposeView();
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Updating justification in fast path: " << justification_MsgNewviewFast.toPrint() << COLOUR_NORMAL << std::endl;
		}
	}

	Validation validation_MsgNewviewFast = Validation();
	if (this->firstFast)
	{
		bool set_Validation = true;
		bool verifier_Validation = true;
		validation_MsgNewviewFast = Validation(set_Validation, verifier_Validation);
	}
	else
	{
		validation_MsgNewviewFast = validation;
	}

	this->isHandleMsgPrepareFast = false;
	this->isHandleMsgPrecommitFast = false;

	// Increase the view
	this->view++;

	// Start the timer
	this->setTimer();

	RoundData roundData_MsgNewviewFast = justification_MsgNewviewFast.getRoundData();
	Phase phase_MsgNewviewFast = roundData_MsgNewviewFast.getPhase();
	Signs signs_MsgNewviewFast = justification_MsgNewviewFast.getSigns();
	if (proposeView_MsgNewviewFast == this->view && phase_MsgNewviewFast == PHASE_NEWVIEW_FAST)
	{
		MsgNewviewFast msgNewviewFast = MsgNewviewFast(roundData_MsgNewviewFast, validation_MsgNewviewFast, signs_MsgNewviewFast);
		if (this->amTrustfailReplicaIds())
		{
			bool isFail_MsgNewviewFast = true;
			msgNewviewFast = MsgNewviewFast(isFail_MsgNewviewFast, roundData_MsgNewviewFast, validation_MsgNewviewFast, signs_MsgNewviewFast);
		}

		if (this->amCurrentLeader())
		{
			this->handleEarlierMessagesFast();
			this->handleMsgNewviewFast(msgNewviewFast);
		}
		else
		{
			ReplicaID leader = this->getCurrentLeader();
			Peers recipients = this->keepFromPeers(leader);
			this->sendMsgNewviewFast(msgNewviewFast, recipients);
			this->handleEarlierMessagesFast();
		}
	}
	else
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Failed to start in fast path" << COLOUR_NORMAL << std::endl;
		}
	}
}

bool ResiBFT::timeToStop()
{
	bool b = this->numViews > 0 && this->numViews <= this->view + 1;
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE
				  << this->printReplicaId()
				  << "timeToStop = " << b
				  << "; numViews = " << this->numViews
				  << "; viewsWithoutNewTrans = " << this->viewsWithoutNewTrans
				  << "; Transaction sizes = " << this->transactions.size()
				  << COLOUR_NORMAL << std::endl;
	}
	if (DEBUG_HELP)
	{
		if (b)
		{
			std::cout << COLOUR_BLUE
					  << this->printReplicaId()
					  << "numViews = " << this->numViews
					  << "; viewsWithoutNewTrans = " << this->viewsWithoutNewTrans
					  << "; Transaction sizes = " << this->transactions.size()
					  << COLOUR_NORMAL << std::endl;
		}
	}
	return b;
}

void ResiBFT::recordStatistics()
{
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "DONE - Printing statistics" << COLOUR_NORMAL << std::endl;
	}

	// Throughput
	double totalView = statistics.getTotalViewTimes();
	unsigned int totalViewNum = statistics.getTotalViewNum();
	double kopsView = (totalViewNum * (NUM_TRANSACTIONS) * 1.0) / 1000;
	double secsView = totalView / (1000 * 1000);
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId()
				  << "VIEW| View = " << this->view
				  << "; Kops = " << kopsView
				  << "; Secs = " << secsView
				  << "; num = " << totalViewNum
				  << COLOUR_NORMAL << std::endl;
	}
	double throughputView = kopsView / secsView;

	// Handle
	double totalHandle = statistics.getTotalHandleTimes();
	unsigned int totalHandleNum = statistics.getTotalHandleNum();
	double kopsHandle = (totalHandleNum * (NUM_TRANSACTIONS) * 1.0) / 1000;
	double secsHandle = totalHandle / (1000 * 1000);
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId()
				  << "HANDLE| View = " << this->view
				  << "; Kops = " << kopsHandle
				  << "; Secs = " << secsHandle
				  << "; num = " << totalHandleNum
				  << COLOUR_NORMAL << std::endl;
	}

	// Latency
	double latencyView = (totalView / totalViewNum / 1000); // milli-seconds spent on views

	// Tentative latency
	double totalViewTentative = statistics.getTotalViewTimesTentative();
	double latencyViewTentative = (totalViewTentative / totalViewNum / 1000); // milli-seconds spent on views

	// Handle
	double handle = (totalHandle / 1000); // milli-seconds spent on handling messages

	std::ofstream fileVals(statisticsValues);
	fileVals << std::to_string(throughputView)
			 << " " << std::to_string(latencyView)
			 << " " << std::to_string(latencyViewTentative)
			 << " " << std::to_string(handle);
	fileVals.close();

	// Done
	std::ofstream fileDone(statisticsDone);
	fileDone.close();
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Printing DONE file: " << statisticsDone << COLOUR_NORMAL << std::endl;
	}

	this->stopped = true;
	this->timer.del();
}

// Constuctor
ResiBFT::ResiBFT(KeysFunctions keysFunctions, ReplicaID replicaId, unsigned int numReplicas, unsigned int numViews, unsigned int numFaults, double leaderChangeTime, Nodes nodes, Key privateKey, std::set<ReplicaID> &generalRecords, std::set<ReplicaID> &trustedRecords, std::set<ReplicaID> &trustfailRecords, PeerNet::Config peerNetConfig, ClientNet::Config clientNetConfig) : peerNet(peerEventContext, peerNetConfig), clientNet(requestEventContext, clientNetConfig)
{
	this->keysFunction = keysFunctions;
	this->replicaId = replicaId;
	this->numReplicas = numReplicas;
	this->numViews = numViews;
	this->numFaults = numFaults;
	this->leaderChangeTime = leaderChangeTime;
	this->nodes = nodes;
	this->privateKey = privateKey;

	this->generalRecords = generalRecords;
	this->trustedRecords = trustedRecords;
	this->trustfailRecords = trustfailRecords;
	this->committee = Committee();
	this->path = COMMON_PATH;
	this->view = 0;
	this->generalQuorumSize = this->numReplicas - this->numFaults;
	this->trustedQuorumSize = ceil((NUM_COMMITTEE_MEMBERS + 1) / 2.0);

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Starting handler" << COLOUR_NORMAL << std::endl;
	}
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "General quorum size: " << this->generalQuorumSize << COLOUR_NORMAL << std::endl;
	}
	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Trusted quorum size: " << this->trustedQuorumSize << COLOUR_NORMAL << std::endl;
	}

	// Trusted Functions
	if (this->amTrustedReplicaIds())
	{
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Initializing TEE" << COLOUR_NORMAL << std::endl;
		}
		this->initializeSGX();
		if (DEBUG_HELP)
		{
			std::cout << COLOUR_BLUE << this->printReplicaId() << "Initialized TEE" << COLOUR_NORMAL << std::endl;
		}
	}
	else
	{
		generalRep = GeneralRep(this->replicaId, this->privateKey, this->generalQuorumSize, this->trustedQuorumSize);
	}

	// Salticidae
	this->requestCall = new salticidae::ThreadCall(this->requestEventContext);

	// The client event context handles replies through [executionQueue]
	this->executionQueue.reg_handler(this->requestEventContext, [this](ExecutionQueue &executionQueue)
									 {
										std::pair<TransactionID,ClientID> transactionPair;
										while (executionQueue.try_dequeue(transactionPair))
										{
											TransactionID transactionId = transactionPair.first;
											ClientID clientId = transactionPair.second;
											Clients::iterator itClient = this->clients.find(clientId);
											if (itClient != this->clients.end())
											{
												ClientInformation clientInformation = itClient->second;
												MsgReply msgReply = MsgReply(transactionId);
												ClientNet::conn_t recipient = std::get<3>(clientInformation);
												if (DEBUG_HELP)
												{
													std::cout << COLOUR_BLUE << this->printReplicaId() << "Sending reply to " << clientId << ": " << msgReply.toPrint() << COLOUR_NORMAL << std::endl;
												}
												try
												{
													this->clientNet.send_msg(msgReply,recipient);
													(this->clients)[clientId]=std::make_tuple(std::get<0>(clientInformation),std::get<1>(clientInformation),std::get<2>(clientInformation)+1,std::get<3>(clientInformation));
												}
												catch(std::exception &error)
												{
													if (DEBUG_HELP)
													{
														std::cout << COLOUR_BLUE << this->printReplicaId() << "Couldn't send reply to " << clientId << ": " << msgReply.toPrint() << "; " << error.what() << COLOUR_NORMAL << std::endl;
													}
												}
											}
											else
											{
												if (DEBUG_HELP)
												{
													std::cout << COLOUR_BLUE << this->printReplicaId() << "Couldn't reply to unknown client: " << clientId << COLOUR_NORMAL << std::endl;
												}
											}
										}
										return false; });

	this->timer = salticidae::TimerEvent(peerEventContext, [this](salticidae::TimerEvent &)
										 {
                                                if (DEBUG_HELP)
												{
													std::cout << COLOUR_BLUE << this->printReplicaId() << "timer ran out" << COLOUR_NORMAL << std::endl;
												}
												if (this->stopped || this->timeToStop())
												{
													this->timer.del();
													return;
												}
												if (this->path == COMMON_PATH)
												{
													this->startNewViewCommon();
												}
												else
											    {
													bool setValidation = true;
													bool verifierValidation = false;
													Validation validation = Validation(setValidation, verifierValidation);
													this->startNewViewFast(validation);
												}
												this->timer.del();
												this->timer.add(this->leaderChangeTime); });

	Host host = "127.0.0.1";
	PortID replicaPort = 8760 + this->replicaId;
	PortID clientPort = 9760 + this->replicaId;

	Node *thisNode = nodes.find(this->replicaId);
	if (thisNode != NULL)
	{
		host = thisNode->getHost();
		replicaPort = thisNode->getReplicaPort();
		clientPort = thisNode->getClientPort();
	}
	else
	{
		std::cout << COLOUR_RED << this->printReplicaId() << "Couldn't find own information among nodes" << COLOUR_NORMAL << std::endl;
	}

	salticidae::NetAddr peerAddress = salticidae::NetAddr(host + ":" + std::to_string(replicaPort));
	this->peerNet.start();
	this->peerNet.listen(peerAddress);

	salticidae::NetAddr clientAddress = salticidae::NetAddr(host + ":" + std::to_string(clientPort));
	this->clientNet.start();
	this->clientNet.listen(clientAddress);

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Connecting..." << COLOUR_NORMAL << std::endl;
	}

	for (size_t j = 0; j < this->numReplicas; j++)
	{
		if (this->replicaId != j)
		{
			Node *otherNode = nodes.find(j);
			if (otherNode != NULL)
			{
				salticidae::NetAddr otherNodeAddress = salticidae::NetAddr(otherNode->getHost() + ":" + std::to_string(otherNode->getReplicaPort()));
				salticidae::PeerId otherPeerId{otherNodeAddress};
				this->peerNet.add_peer(otherPeerId);
				this->peerNet.set_peer_addr(otherPeerId, otherNodeAddress);
				this->peerNet.conn_peer(otherPeerId);
				if (DEBUG_HELP)
				{
					std::cout << COLOUR_BLUE << this->printReplicaId() << "Added peer: " << j << COLOUR_NORMAL << std::endl;
				}
				this->peers.push_back(std::make_pair(j, otherPeerId));
			}
			else
			{
				std::cout << COLOUR_RED << this->printReplicaId() << "Couldn't find " << j << "'s information among nodes" << COLOUR_NORMAL << std::endl;
			}
		}
	}

	this->clientNet.reg_handler(salticidae::generic_bind(&ResiBFT::receiveMsgStart, this, _1, _2));
	this->clientNet.reg_handler(salticidae::generic_bind(&ResiBFT::receiveMsgTransaction, this, _1, _2));

	this->peerNet.reg_handler(salticidae::generic_bind(&ResiBFT::receiveMsgNewviewCommon, this, _1, _2));
	this->peerNet.reg_handler(salticidae::generic_bind(&ResiBFT::receiveMsgLdrprepareCommon, this, _1, _2));
	this->peerNet.reg_handler(salticidae::generic_bind(&ResiBFT::receiveMsgPrepareCommon, this, _1, _2));
	this->peerNet.reg_handler(salticidae::generic_bind(&ResiBFT::receiveMsgPrecommitCommon, this, _1, _2));
	this->peerNet.reg_handler(salticidae::generic_bind(&ResiBFT::receiveMsgCommitCommon, this, _1, _2));

	this->peerNet.reg_handler(salticidae::generic_bind(&ResiBFT::receiveMsgNewviewFast, this, _1, _2));
	this->peerNet.reg_handler(salticidae::generic_bind(&ResiBFT::receiveMsgLdrprepareFast, this, _1, _2));
	this->peerNet.reg_handler(salticidae::generic_bind(&ResiBFT::receiveMsgPrepareFast, this, _1, _2));
	this->peerNet.reg_handler(salticidae::generic_bind(&ResiBFT::receiveMsgPrecommitFast, this, _1, _2));
	this->peerNet.reg_handler(salticidae::generic_bind(&ResiBFT::receiveMsgValidationFast, this, _1, _2));

	this->peerNet.reg_handler(salticidae::generic_bind(&ResiBFT::receiveMsgLdrprepareFast2Common, this, _1, _2));
	this->peerNet.reg_handler(salticidae::generic_bind(&ResiBFT::receiveMsgPrepareFast2Common, this, _1, _2));
	this->peerNet.reg_handler(salticidae::generic_bind(&ResiBFT::receiveMsgPrecommitFast2Common, this, _1, _2));
	this->peerNet.reg_handler(salticidae::generic_bind(&ResiBFT::receiveMsgCommitFast2Common, this, _1, _2));

	// Statistics
	auto timeNow = std::chrono::system_clock::now();
	std::time_t time = std::chrono::system_clock::to_time_t(timeNow);
	struct tm y2k = {0};
	double seconds = difftime(time, mktime(&y2k));
	statisticsValues = "results/values-" + std::to_string(this->replicaId) + "-" + std::to_string(seconds);
	statisticsDone = "results/done-" + std::to_string(this->replicaId) + "-" + std::to_string(seconds);
	statistics.setReplicaId(this->replicaId);

	auto peerShutDown = [&](int)
	{ peerEventContext.stop(); };
	salticidae::SigEvent peerSigTerm = salticidae::SigEvent(peerEventContext, peerShutDown);
	peerSigTerm.add(SIGTERM);

	auto clientShutDown = [&](int)
	{ requestEventContext.stop(); };
	salticidae::SigEvent clientSigTerm = salticidae::SigEvent(requestEventContext, clientShutDown);
	clientSigTerm.add(SIGTERM);

	requestThread = std::thread([this]()
								{ requestEventContext.dispatch(); });

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Dispatching request thread" << COLOUR_NORMAL << std::endl;
	}

	peerEventContext.dispatch();

	if (DEBUG_HELP)
	{
		std::cout << COLOUR_BLUE << this->printReplicaId() << "Dispatching peer thread" << COLOUR_NORMAL << std::endl;
	}
}