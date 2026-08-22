#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "message.h"
#include "types.h"
#include "KeysFunctions.h"
#include "Nodes.h"
#include "RoundData.h"
#include "Signs.h"
#include "ResiBFT.h"

unsigned int replicaId;
unsigned int numGeneralReplicas;
unsigned int numTrustedReplicas;
unsigned int numTrustfailReplicas;
unsigned int numReplicas;
unsigned int numViews;
unsigned int numFaults;
double leaderChangeTime;
std::set<ReplicaID> generalRecords;
std::set<ReplicaID> trustedRecords;
std::set<ReplicaID> trustfailRecords;

std::string printReplicaId()
{
	return ("[SERVER" + std::to_string(replicaId) + "]");
}

int main(int argc, char const *argv[])
{
	KeysFunctions keysFunctions;

	// Get inputs
	if (DEBUG_SERVER)
	{
		std::cout << COLOUR_ORANGE << "Parsing server inputs" << COLOUR_NORMAL << std::endl;
	}

	if (argc > 1)
	{
		sscanf(argv[1], "%d", &replicaId);
	}
	std::cout << COLOUR_ORANGE << printReplicaId() << "replicaId = " << replicaId << COLOUR_NORMAL << std::endl;

	if (argc > 2)
	{
		sscanf(argv[2], "%d", &numGeneralReplicas);
	}
	std::cout << COLOUR_ORANGE << printReplicaId() << "numGeneralReplicas = " << numGeneralReplicas << COLOUR_NORMAL << std::endl;

	generalRecords.clear();
	if (argc > 3)
	{
		std::string generalRecordsStr = argv[3];
		std::stringstream ss(generalRecordsStr);
		std::string item;

		while (std::getline(ss, item, ','))
		{
			if (!item.empty())
			{
				ReplicaID generalReplicaId;
				sscanf(item.c_str(), "%d", &generalReplicaId);
				generalRecords.insert(generalReplicaId);
			}
		}
	}

	std::cout << COLOUR_ORANGE << printReplicaId() << "generalRecords=[";
	for (std::set<ReplicaID>::iterator it = generalRecords.begin(); it != generalRecords.end(); it++)
	{
		if (it != generalRecords.begin())
		{
			std::cout << ",";
		}
		std::cout << *it;
	}
	std::cout << "]" << COLOUR_NORMAL << std::endl;

	if (generalRecords.size() != numGeneralReplicas)
	{
		std::cout << COLOUR_RED << printReplicaId() << "Invalid general records size. Expected " << numGeneralReplicas << ", but got " << generalRecords.size() << COLOUR_NORMAL << std::endl;
	}

	trustedRecords.clear();
	if (argc > 4)
	{
		sscanf(argv[4], "%d", &numTrustedReplicas);
	}
	std::cout << COLOUR_ORANGE << printReplicaId() << "numTrustedReplicas = " << numTrustedReplicas << COLOUR_NORMAL << std::endl;

	if (argc > 5)
	{
		std::string trustedRecordsStr = argv[5];
		std::stringstream ss(trustedRecordsStr);
		std::string item;

		while (std::getline(ss, item, ','))
		{
			if (!item.empty())
			{
				ReplicaID trustedReplicaId;
				sscanf(item.c_str(), "%d", &trustedReplicaId);
				trustedRecords.insert(trustedReplicaId);
			}
		}
	}

	std::cout << COLOUR_ORANGE << printReplicaId() << "trustedRecords=[";
	for (std::set<ReplicaID>::iterator it = trustedRecords.begin(); it != trustedRecords.end(); it++)
	{
		if (it != trustedRecords.begin())
		{
			std::cout << ",";
		}
		std::cout << *it;
	}
	std::cout << "]" << COLOUR_NORMAL << std::endl;

	if (trustedRecords.size() != numTrustedReplicas)
	{
		std::cout << COLOUR_RED << printReplicaId() << "Invalid trusted records size. Expected " << numTrustedReplicas << ", but got " << trustedRecords.size() << COLOUR_NORMAL << std::endl;
	}

	trustfailRecords.clear();
	if (argc > 6)
	{
		sscanf(argv[6], "%d", &numTrustfailReplicas);
	}
	std::cout << COLOUR_ORANGE << printReplicaId() << "numTrustfailReplicas = " << numTrustfailReplicas << COLOUR_NORMAL << std::endl;

	if (argc > 7)
	{
		if (numTrustfailReplicas > 0)
		{
			std::string trustfailRecordsStr = argv[7];
			std::stringstream ss(trustfailRecordsStr);
			std::string item;

			while (std::getline(ss, item, ','))
			{
				if (!item.empty())
				{
					ReplicaID trustfailReplicaId;
					sscanf(item.c_str(), "%d", &trustfailReplicaId);
					trustfailRecords.insert(trustfailReplicaId);
				}
			}
		}
	}

	std::cout << COLOUR_ORANGE << printReplicaId() << "trustfailRecords=[";
	for (std::set<ReplicaID>::iterator it = trustfailRecords.begin(); it != trustfailRecords.end(); it++)
	{
		if (it != trustfailRecords.begin())
		{
			std::cout << ",";
		}
		std::cout << *it;
	}
	std::cout << "]" << COLOUR_NORMAL << std::endl;

	if (trustfailRecords.size() != numTrustfailReplicas)
	{
		std::cout << COLOUR_RED << printReplicaId() << "Invalid trust-fail records size. Expected " << numTrustfailReplicas << ", but got " << trustfailRecords.size() << COLOUR_NORMAL << std::endl;
	}

	if (argc > 8)
	{
		sscanf(argv[8], "%d", &numReplicas);
	}
	std::cout << COLOUR_ORANGE << printReplicaId() << "numReplicas = " << numReplicas << COLOUR_NORMAL << std::endl;

	if (generalRecords.size() + trustedRecords.size() != numReplicas)
	{
		std::cout << COLOUR_RED << printReplicaId() << "Invalid numReplicas size." << COLOUR_NORMAL << std::endl;
	}

	if (argc > 9)
	{
		sscanf(argv[9], "%d", &numViews);
	}
	std::cout << COLOUR_ORANGE << printReplicaId() << "numViews = " << numViews << COLOUR_NORMAL << std::endl;

	if (argc > 10)
	{
		sscanf(argv[10], "%d", &numFaults);
	}
	std::cout << COLOUR_ORANGE << printReplicaId() << "numFaults = " << numFaults << COLOUR_NORMAL << std::endl;

	if (argc > 11)
	{
		sscanf(argv[11], "%lf", &leaderChangeTime);
	}
	std::cout << COLOUR_ORANGE << printReplicaId() << "leaderChangeTime = " << leaderChangeTime << COLOUR_NORMAL << std::endl;

	std::string configFile = "config";
	Nodes nodes(configFile, numReplicas);

	// Private key
	Key privateKey;
	BIO *bio = BIO_new(BIO_s_mem());
	int w = BIO_write(bio, priv_key256, sizeof(priv_key256));
	privateKey = PEM_read_bio_ECPrivateKey(bio, NULL, NULL, NULL);

	if (DEBUG_SERVER)
	{
		std::cout << COLOUR_ORANGE << printReplicaId() << "Checking private key" << COLOUR_NORMAL << std::endl;
	}
	if (!EC_KEY_check_key(privateKey))
	{
		std::cout << COLOUR_ORANGE << printReplicaId() << "Invalid private key" << COLOUR_NORMAL << std::endl;
	}
	if (DEBUG_SERVER)
	{
		std::cout << COLOUR_ORANGE << printReplicaId() << "Checked private key" << COLOUR_NORMAL << std::endl;
	}

	// Public keys
	for (unsigned int i = 0; i < numReplicas; i++)
	{
		Key publicKey;
		BIO *bio = BIO_new(BIO_s_mem());
		int w = BIO_write(bio, pub_key256, sizeof(pub_key256));
		publicKey = PEM_read_bio_EC_PUBKEY(bio, NULL, NULL, NULL);
		if (DEBUG_SERVER)
		{
			std::cout << COLOUR_ORANGE << printReplicaId() << "Replica id: " << i << COLOUR_NORMAL << std::endl;
		}
		nodes.setPublicKey(i, publicKey);
	}

	// Initialize handler
	if (DEBUG_SERVER)
	{
		std::cout << COLOUR_ORANGE << printReplicaId() << "Initializing handler" << COLOUR_NORMAL << std::endl;
	}

	long unsigned int sizeBasic = std::max({sizeof(MsgTransaction), sizeof(MsgReply), sizeof(MsgStart)});
	long unsigned int sizeMessage = std::max({sizeof(MsgNewviewCommon), sizeof(MsgLdrprepareCommon), sizeof(MsgPrepareCommon), sizeof(MsgPrecommitCommon), sizeof(MsgCommitCommon), sizeof(MsgNewviewFast), sizeof(MsgLdrprepareFast), sizeof(MsgPrepareFast), sizeof(MsgPrecommitFast), sizeof(MsgValidationFast)});
	long unsigned int size = std::max({sizeBasic, sizeMessage});

	if (DEBUG_SERVER)
	{
		std::cout << COLOUR_ORANGE << printReplicaId() << "Max msg size = " << size << COLOUR_NORMAL << std::endl;
	}

	PeerNet::Config peerNetConfig;
	peerNetConfig.max_msg_size(2 * size);
	ClientNet::Config clientNetConfig;
	clientNetConfig.max_msg_size(2 * size);
	if (DEBUG_SERVER)
	{
		std::cout << COLOUR_ORANGE << printReplicaId() << "Starting handler" << COLOUR_NORMAL << std::endl;
	}

	ResiBFT resiBFT(keysFunctions, replicaId, numReplicas, numViews, numFaults, leaderChangeTime, nodes, privateKey, generalRecords, trustedRecords, trustfailRecords, peerNetConfig, clientNetConfig);

	return 0;
};
