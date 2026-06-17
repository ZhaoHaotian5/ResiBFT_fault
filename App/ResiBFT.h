#ifndef RESIBFT_H
#define RESIBFT_H

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <math.h>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <random>
#include <thread>
#include <unistd.h>
#include <vector>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <sys/socket.h>
#include "../Enclave/EnclaveUsertypes.h"
#include "salticidae/event.h"
#include "salticidae/msg.h"
#include "salticidae/network.h"
#include "salticidae/stream.h"
#include "sgx_urts.h"
#include "sgx_utils/sgx_utils.h"
#include "Enclave_u.h"
#include "Committee.h"
#include "Nodes.h"
#include "KeysFunctions.h"
#include "Log.h"
#include "ProposalCommon.h"
#include "ProposalFast.h"
#include "GeneralRep.h"
#include "Statistics.h"
#include "Transaction.h"

using std::placeholders::_1;
using std::placeholders::_2;
using Peer = std::tuple<ReplicaID, salticidae::PeerId>;
using Peers = std::vector<Peer>;
using PeerNet = salticidae::PeerNetwork<uint8_t>;
using ClientNet = salticidae::ClientNetwork<uint8_t>;
using ClientInformation = std::tuple<bool, unsigned int, unsigned int, ClientNet::conn_t>; // [bool] is true if the client hasn't stopped, 1st [int] is the number of transactions received from clients, 2nd [int] is the number of transactions replied to
using Clients = std::map<ClientID, ClientInformation>;
using MessageNet = salticidae::MsgNetwork<uint8_t>;
using ExecutionQueue = salticidae::MPSCQueueEventDriven<std::pair<TransactionID, ClientID>>;
using Time = std::chrono::time_point<std::chrono::steady_clock>;

class ResiBFT
{
private:
	// Basic settings
	KeysFunctions keysFunction;
	ReplicaID replicaId;
	unsigned int numReplicas;
	unsigned int numViews;
	unsigned int numFaults;
	double leaderChangeTime;
	Nodes nodes;
	Key privateKey;

	// Basic data
	std::set<ReplicaID> generalRecords;
	std::set<ReplicaID> trustedRecords;
	std::set<ReplicaID> trustfailRecords;
	unsigned int numGeneralReplicas;
	Quorum generalQuorumSize;
	Quorum trustedQuorumSize;
	Committee committee;
	Path path;
	View view;

	// Message handlers
	salticidae::EventContext peerEventContext;	  // Peer event context
	salticidae::EventContext requestEventContext; // Request event context
	Peers peers;
	PeerNet peerNet;
	Clients clients;
	ClientNet clientNet;
	std::thread requestThread;
	salticidae::BoxObj<salticidae::ThreadCall> requestCall;
	salticidae::TimerEvent timer;
	ExecutionQueue executionQueue;

	// State variables
	std::list<Transaction> transactions;	 // Current transactions waiting to be processed
	std::map<View, Block> blocks;			 // Blocks received in each view
	std::map<View, Validations> validations; // Validations received in each view
	std::mutex mutexTransaction;
	std::mutex mutexHandle;
	unsigned int viewsWithoutNewTrans = 0;
	bool started = false;
	bool stopped = false;
	bool firstFast = false;
	bool isHandleMsgPrepareFast = false;
	bool isHandleMsgPrecommitFast = false;
	View timerView; // View at which the timer was started
	Log log;

	// Print functions
	std::string printReplicaId();
	void printNowTime(std::string msg);
	void printClientInfo();
	std::string recipients2string(Peers recipients);

	// Setting functions
	bool amGeneralReplicaIds();
	bool isGeneralReplicaIds(ReplicaID replicaId);
	bool amTrustedReplicaIds();
	bool isTrustedReplicaIds(ReplicaID replicaId);
	bool amTrustfailReplicaIds();
	bool amCommitteeReplicaIds();

	unsigned int getLeaderOf(View view);
	unsigned int getCurrentLeader();
	bool amLeaderOf(View view);
	bool amCurrentLeader();

	Peers removeFromPeers(ReplicaID replicaId);
	Peers removeFromCommitteePeers(Committee committee, ReplicaID replicaId);
	Peers keepFromPeers(ReplicaID replicaId);
	Peers keepFromCommitteePeers(Committee committee, ReplicaID replicaId);
	std::vector<salticidae::PeerId> getPeerIds(Peers recipients);
	void setTimer();

	Committee selectRandomCommittee(std::set<ReplicaID> trustedRecords);
	Block createNewBlock(Hash hash);
	void executeBlockCommon(RoundData roundData_MsgCommitCommon);
	void executeBlockFast(RoundData roundData_MsgPrecommit, Validation validation);

	// Reply to clients
	void replyTransactions(Transaction *transactions);
	void replyHash(Hash hash);

	// Call TEE functions
	bool verifyJustification(Justification justification);
	bool verifyProposalCommon(ProposalCommon proposalCommon, Signs signs);
	bool verifyProposalFast(ProposalFast proposalFast, Signs signs);

	Justification initializeMsgNewviewCommon();
	Justification respondProposalCommon(Hash proposeHash, Justification justification_MsgNewviewCommon);
	Signs initializeMsgLdrprepareCommon(ProposalCommon proposal_MsgLdrprepareCommon);
	Justification saveMsgPrepareCommon(Justification justification_MsgPrepareCommon);
	Justification lockMsgPrecommitCommon(Justification justification_MsgPrecommitCommon);

	Justification initializeMsgNewviewFast();
	Accumulator initializeAccumulatorFast(Justification justifications_MsgNewview[NUM_ACTIVE_REPLICAS]);
	Justification respondProposalFast(Hash proposeHash, Accumulator accumulator_MsgLdrprepareFast);
	Signs initializeMsgLdrprepareFast(ProposalFast proposal_MsgLdrprepare);
	Justification saveMsgPrepareFast(Justification justification_MsgPrepareFast, bool isFail_MsgPrepareFast);

	Justification initializeMsgNewviewFast2Common(Justification justification_MsgNewviewFast);
	Justification respondProposalFast2Common(Hash proposeHash, Justification justification_MsgNewviewFast);
	Signs initializeMsgLdrprepareFast2Common(ProposalCommon proposal_MsgLdrprepareFast2Common);
	Justification saveMsgPrepareFast2Common(Justification justification_MsgPrepareFast2Common);
	Justification lockMsgPrecommitFast2Common(Justification justification_MsgPrecommitFast2Common);

	Validations buildValidations(std::set<MsgNewviewFast> msgNewviewFasts);
	Accumulator buildAccumulator(std::set<MsgNewviewFast> msgNewviewFasts);

	// Change path
	void common2fast();
	void fast2common();
	void initializeCheckpoint(Hash proposeHash, View proposeView);
	void updateCheckpoint(Hash verifyHash, View verifyView, Validations validations, Signs signs);
	Validation checkBlock(Justification justification, bool isFail);

	// Receive messages
	void receiveMsgStart(MsgStart msgStart, const ClientNet::conn_t &conn);
	void receiveMsgTransaction(MsgTransaction msgTransaction, const ClientNet::conn_t &conn);

	void receiveMsgNewviewCommon(MsgNewviewCommon msgNewviewCommon, const PeerNet::conn_t &conn);
	void receiveMsgLdrprepareCommon(MsgLdrprepareCommon msgLdrprepareCommon, const PeerNet::conn_t &conn);
	void receiveMsgPrepareCommon(MsgPrepareCommon msgPrepareCommon, const PeerNet::conn_t &conn);
	void receiveMsgPrecommitCommon(MsgPrecommitCommon msgPrecommitCommon, const PeerNet::conn_t &conn);
	void receiveMsgCommitCommon(MsgCommitCommon msgCommitCommon, const PeerNet::conn_t &conn);

	void receiveMsgNewviewFast(MsgNewviewFast msgNewviewFast, const PeerNet::conn_t &conn);
	void receiveMsgLdrprepareFast(MsgLdrprepareFast msgLdrprepareFast, const PeerNet::conn_t &conn);
	void receiveMsgPrepareFast(MsgPrepareFast msgPrepareFast, const PeerNet::conn_t &conn);
	void receiveMsgPrecommitFast(MsgPrecommitFast msgPrecommitFast, const PeerNet::conn_t &conn);
	void receiveMsgValidationFast(MsgValidationFast msgValidationFast, const PeerNet::conn_t &conn);

	void receiveMsgLdrprepareFast2Common(MsgLdrprepareFast2Common msgLdrprepareFast2Common, const PeerNet::conn_t &conn);
	void receiveMsgPrepareFast2Common(MsgPrepareFast2Common msgPrepareFast2Common, const PeerNet::conn_t &conn);
	void receiveMsgPrecommitFast2Common(MsgPrecommitFast2Common msgPrecommitFast2Common, const PeerNet::conn_t &conn);
	void receiveMsgCommitFast2Common(MsgCommitFast2Common msgCommitFast2Common, const PeerNet::conn_t &conn);

	// Send messages
	void sendMsgNewviewCommon(MsgNewviewCommon msgNewviewCommon, Peers recipients);
	void sendMsgLdrprepareCommon(MsgLdrprepareCommon msgLdrprepareCommon, Peers recipients);
	void sendMsgPrepareCommon(MsgPrepareCommon msgPrepareCommon, Peers recipients);
	void sendMsgPrecommitCommon(MsgPrecommitCommon msgPrecommitCommon, Peers recipients);
	void sendMsgCommitCommon(MsgCommitCommon msgCommitCommon, Peers recipients);

	void sendMsgNewviewFast(MsgNewviewFast msgNewviewFast, Peers recipients);
	void sendMsgLdrprepareFast(MsgLdrprepareFast msgLdrprepareFast, Peers recipients);
	void sendMsgPrepareFast(MsgPrepareFast msgPrepareFast, Peers recipients);
	void sendMsgPrecommitFast(MsgPrecommitFast msgPrecommitFast, Peers recipients);
	void sendMsgValidationFast(MsgValidationFast msgValidationFast, Peers recipients);

	void sendMsgLdrprepareFast2Common(MsgLdrprepareFast2Common msgLdrprepareFast2Common, Peers recipients);
	void sendMsgPrepareFast2Common(MsgPrepareFast2Common msgPrepareFast2Common, Peers recipients);
	void sendMsgPrecommitFast2Common(MsgPrecommitFast2Common msgPrecommitFast2Common, Peers recipients);
	void sendMsgCommitFast2Common(MsgCommitFast2Common msgCommitFast2Common, Peers recipients);

	// Handle messages
	void handleMsgTransaction(MsgTransaction msgTransaction);
	void handleEarlierMessagesCommon(); // For replicas to process messages they have already received for in new view
	void handleEarlierMessagesFast();	// For replicas to process messages they have already received for in new view

	void handleMsgNewviewCommon(MsgNewviewCommon msgNewviewCommon);			 // Once the leader has received [msgNewviewCommon], it creates [msgLdrprepareCommon] out of the highest prepared block
	void handleMsgLdrprepareCommon(MsgLdrprepareCommon MsgLdrprepareCommon); // Once the replicas have received [msgLdrprepareCommon], it creates [msgPrepareCommon] out of the proposal
	void handleMsgPrepareCommon(MsgPrepareCommon msgPrepareCommon);			 // For both the leader and replicas process [msgPrepareCommon]
	void handleMsgPrecommitCommon(MsgPrecommitCommon msgPrecommitCommon);	 // For both the leader and replicas process [msgPrecommitCommon]
	void handleMsgCommitCommon(MsgCommitCommon msgCommitCommon);			 // For both the leader and replicas process [msgCommitCommon]

	void handleMsgNewviewFast(MsgNewviewFast msgNewviewFast);		   // Once the leader has received [msgNewviewFast], it creates [msgLdrprepareFast] out of the highest prepared block
	void handleMsgLdrprepareFast(MsgLdrprepareFast msgLdrprepareFast); // Once the replicas have received [msgLdrprepareFast], it creates [msgPrepareFast] out of the proposal
	void handleMsgPrepareFast(MsgPrepareFast msgPrepareFast);		   // For both the leader and replicas process [msgPrepareFast]
	void handleMsgPrecommitFast(MsgPrecommitFast msgPrecommitFast);	   // For both the leader and replicas process [msgPrecommitFast]
	void handleMsgValidationFast(MsgValidationFast msgValidationFast); // For the non-committee replicas process [msgValidationFast]

	void handleMsgLdrprepareFast2Common(MsgLdrprepareFast2Common MsgLdrprepareFast2Common); // Once the replicas have received [msgLdrprepareFast2Common], it creates [msgPrepareFast2Common] out of the proposal
	void handleMsgPrepareFast2Common(MsgPrepareFast2Common msgPrepareFast2Common);			// For both the leader and replicas process [msgPrepareFast2Common]
	void handleMsgPrecommitFast2Common(MsgPrecommitFast2Common msgPrecommitFast2Common);	// For both the leader and replicas process [msgPrecommitFast2Common]
	void handleMsgCommitFast2Common(MsgCommitFast2Common msgCommitFast2Common);				// For both the leader and replicas process [msgCommitFast2Common]

	// Initiate messages
	void initiateMsgNewviewCommon();									 // Leader send [msgLdrprepareCommon] to others and hold its own [msgPrepareCommon]
	void initiateMsgPrepareCommon(RoundData roundData_MsgPrepareCommon); // Leader send [msgPrepareCommon] to others and hold its own [msgPrecommitCommon]
	void initiateMsgPrecommitCommon(RoundData roundData_MsgPrecommit);	 // Leader send [msgPrecommitCommon] to others and hold its own [msgCommitCommon]
	void initiateMsgCommitCommon(RoundData roundData_MsgCommit);		 // Leader send [msgCommitCommon] to others and execute the block

	void initiateMsgNewviewFast();										 // Leader send [msgLdrprepareFast] to others and hold its own [msgPrepareFast]
	void initiateMsgPrepareFast(RoundData roundData_MsgPrepareFast);	 // Leader send [msgPrepareFast] to others and hold its own [msgPrecommitFast]
	void initiateMsgPrecommitFast(RoundData roundData_MsgPrecommitFast); // Leader send [msgPrecommitFast] to others and execute the block

	void initiateMsgPrepareFast2Common(RoundData roundData_MsgPrepareFast2Common); // Leader send [msgPrepareFast2Common] to others and hold its own [msgPrecommitFast2Common]
	void initiateMsgPrecommitFast2Common(RoundData roundData_MsgPrecommit);		   // Leader send [msgPrecommitFast2Common] to others and hold its own [msgCommitFast2Common]
	void initiateMsgCommitFast2Common(RoundData roundData_MsgCommit);			   // Leader send [msgCommitFast2Common] to others and execute the block

	// Respond messages
	void respondMsgLdrprepareCommon(Justification justification_MsgNewviewCommon, Committee committee_MsgLdrprepareCommon, Block block); // Replicas respond to [msgLdrprepareCommon] and send [msgPrepareCommon] to the leader
	void respondMsgPrepareCommon(Justification justification_MsgPrepare);																 // Replicas respond to [msgPrepareCommon] and send [msgPrecommitCommon] to the leader
	void respondMsgPrecommitCommon(Justification justification_MsgPrecommitCommon);														 // Replicas respond to [msgPrecommitCommon] and send [msgCommitCommon] to the leader

	void respondMsgLdrprepareFast(Accumulator accumulator_MsgLdrprepareFast, Validations validations_MsgLdrprepareFast, Block block); // Replicas respond to [msgLdrprepareFast] and send [msgPrepareFast] to the leader
	void respondMsgPrepareFast(Justification justification_MsgPrepareFast, bool isFail_MsgPrepareFast);								  // Replicas respond to [msgPrepareFast] and send [msgPrecommitFast] to the leader

	void respondMsgLdrprepareFast2Common(Justification justification_MsgNewviewFast2Common, Committee committee_MsgLdrprepareFast2Common, Block block, ReplicaID singer_MsgLdrprepareFast2Common); // Replicas respond to [msgLdrprepareFast2Common] and send [msgPrepareFast2Common] to the leader
	void respondMsgPrepareFast2Common(Justification justification_MsgPrepareFast2Common, ReplicaID singer_MsgPrepareFast2Common);																   // Replicas respond to [msgPrepareFast2Common] and send [msgPrecommitFast2Common] to the leader
	void respondMsgPrecommitFast2Common(Justification justification_MsgPrecommitFast2Common, ReplicaID singer_MsgPrecommitFast2Common);															   // Replicas respond to [msgPrecommitFast2Common] and send [msgCommitFast2Common] to the leader

	// Main functions
	int initializeSGX();
	void getStartedCommon();
	void startNewViewCommon();
	void startNewViewFast(Validation validation);
	bool timeToStop();
	void recordStatistics();

public:
	ResiBFT(KeysFunctions keysFunctions, ReplicaID replicaId, unsigned int numReplicas, unsigned int numViews, unsigned int numFaults, double leaderChangeTime, Nodes nodes, Key privateKey, std::set<ReplicaID> &generalRecords, std::set<ReplicaID> &trustedRecords, std::set<ReplicaID> &trustfailRecords, PeerNet::Config peerNetConfig, ClientNet::Config clientNetConfig);
};

#endif
