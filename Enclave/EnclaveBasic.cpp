#include "EnclaveBasic.h"

Quorum generalQuorumSize_t;			 // General quorum size
Quorum trustedQuorumSize_t;			 // Trusted quorum size
ReplicaID replicaId;				 // Unique identifier
Key privateKey;						 // Private key
std::map<ReplicaID, Key> publicKeys; // Public keys

Quorum getGeneralQuorumSize_t()
{
	return generalQuorumSize_t;
}

Quorum getTrustedQuorumSize_t()
{
	return trustedQuorumSize_t;
}

std::string printReplicaId_t()
{
	std::string text = "[" + std::to_string(replicaId) + "]";
	return text;
}

void loadPrivateKey_t()
{
	BIO *bio = BIO_new(BIO_s_mem());
	int w = BIO_write(bio, priv_key256, sizeof(priv_key256));
	privateKey = PEM_read_bio_ECPrivateKey(bio, NULL, NULL, NULL);

	if (DEBUG_TEE)
	{
		TEE_Print((printReplicaId_t() + "ENCLAVE: Checking private key").c_str());
	}
	if (DEBUG_TEE)
	{
		if (!EC_KEY_check_key(privateKey))
		{
			TEE_Print((printReplicaId_t() + "ENCLAVE: Invalid private key").c_str());
		}
	}
	if (DEBUG_TEE)
	{
		TEE_Print((printReplicaId_t() + "ENCLAVE: Checked private key").c_str());
	}
}

sgx_status_t TEE_initializeVariables(ReplicaID *me, Pids_t *others, unsigned int *GeneralQuorumSize, unsigned int *TrustedQuorumSize)
{
	sgx_status_t status_t = SGX_SUCCESS;
	replicaId = *me;
	TEE_Print((printReplicaId_t() + "ENCLAVE: Set up replicaId " + std::to_string(replicaId)).c_str());
	generalQuorumSize_t = *GeneralQuorumSize;
	TEE_Print((printReplicaId_t() + "ENCLAVE: Set up the general quorum size " + std::to_string(generalQuorumSize_t)).c_str());
	trustedQuorumSize_t = *TrustedQuorumSize;
	TEE_Print((printReplicaId_t() + "ENCLAVE: Set up the trusted quorum size " + std::to_string(trustedQuorumSize_t)).c_str());
	loadPrivateKey_t();
	for (int i = 0; i < others->num_nodes; i++)
	{
		ReplicaID j = others->pids[i];
		BIO *bio = BIO_new(BIO_s_mem());
		int w = BIO_write(bio, pub_key256, sizeof(pub_key256));
		Key publicKey = PEM_read_bio_EC_PUBKEY(bio, NULL, NULL, NULL);
		if (!EC_KEY_check_key(publicKey))
		{
			TEE_Print((printReplicaId_t() + "ENCLAVE: Invalid public key").c_str());
		}
		publicKeys[j] = publicKey;
	}
	TEE_Print((printReplicaId_t() + "ENCLAVE: Loaded public keys").c_str());

	return status_t;
}

std::string transaction2string_t(Transaction_t transaction_t)
{
	std::string text = "";
	text += std::to_string(transaction_t.clientId);
	text += std::to_string(transaction_t.transactionId);
	for (int i = 0; i < NUM_PAYLOAD_SIZE; i++)
	{
		text += transaction_t.transactionData[i];
	}
	return text;
}

std::string hash2string_t(Hash_t hash_t)
{
	std::string text = "";
	text += std::to_string(hash_t.set);
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		text += hash_t.hash[i];
	}
	return text;
}

std::string block2string_t(Block_t block_t)
{
	std::string text = "";
	text += std::to_string(block_t.set);
	text += hash2string_t(block_t.previousHash);
	text += std::to_string(block_t.size);
	for (int i = 0; i < NUM_TRANSACTIONS; i++)
	{
		text += transaction2string_t(block_t.transactions[i]);
	}
	return text;
}

std::string roundData2string_t(RoundData_t roundData_t)
{
	std::string text = "";
	text += hash2string_t(roundData_t.proposeHash);
	text += std::to_string(roundData_t.proposeView);
	text += hash2string_t(roundData_t.justifyHash);
	text += std::to_string(roundData_t.justifyView);
	text += std::to_string(roundData_t.phase);
	return text;
}

std::string sign2string_t(Sign_t sign_t)
{
	std::string text = "";
	text += std::to_string(sign_t.set);
	text += std::to_string(sign_t.signer);
	for (int i = 0; i < SIGN_LEN; i++)
	{
		text += sign_t.signtext[i];
	}
	return text;
}

std::string signs2string_t(Signs_t signs_t)
{
	std::string text = "";
	text += std::to_string(signs_t.size);
	for (int i = 0; i < NUM_ACTIVE_REPLICAS; i++)
	{
		text += sign2string_t(signs_t.signs[i]);
	}
	return text;
}

std::string justification2string_t(Justification_t justification_t)
{
	std::string text = "";
	text += std::to_string(justification_t.set);
	text += roundData2string_t(justification_t.roundData);
	text += signs2string_t(justification_t.signs);
	return text;
}

std::string justifications2string_t(Justifications_t justifications_t)
{
	std::string text = "";
	for (int i = 0; i < NUM_ACTIVE_REPLICAS; i++)
	{
		text += justification2string_t(justifications_t.justifications[i]);
	}
	return text;
}

std::string accumulator2string_t(Accumulator_t accumulator_t)
{
	std::string text = "";
	text += std::to_string(accumulator_t.set);
	text += std::to_string(accumulator_t.proposeView);
	text += hash2string_t(accumulator_t.prepareHash);
	text += std::to_string(accumulator_t.prepareView);
	text += std::to_string(accumulator_t.size);
	return text;
}

std::string proposalCommon2string_t(ProposalCommon_t proposalCommon_t)
{
	std::string text = "";
	text += justification2string_t(proposalCommon_t.justification);
	text += block2string_t(proposalCommon_t.block);
	return text;
}

std::string proposalFast2string_t(ProposalFast_t proposalFast_t)
{
	std::string text = "";
	text += accumulator2string_t(proposalFast_t.accumulator);
	text += block2string_t(proposalFast_t.block);
	return text;
}

std::string validation2string_t(Validation_t validation_t)
{
	std::string text = "";
	text += std::to_string(validation_t.set);
	text += std::to_string(validation_t.verifier);
	return text;
}

std::string validations2string_t(Validations_t validations_t)
{
	std::string text = "";
	text += std::to_string(validations_t.size);
	for (int i = 0; i < NUM_ACTIVE_REPLICAS; i++)
	{
		text += validation2string_t(validations_t.validations[i]);
	}
	return text;
}

std::string certificate2string_t(Certificate_t certificate_t)
{
	std::string text = "";
	text += block2string_t(certificate_t.block);
	text += validations2string_t(certificate_t.validations);
	text += signs2string_t(certificate_t.signs);
	return text;
}

std::string checkpoint2string_t(Checkpoint_t checkpoint_t)
{
	std::string text = "";
	text += hash2string_t(checkpoint_t.verifyHash);
	text += std::to_string(checkpoint_t.verifyView);
	text += validations2string_t(checkpoint_t.validations);
	text += signs2string_t(checkpoint_t.signs);
	return text;
}

bool equalHashes_t(Hash_t hash1, Hash_t hash2)
{
	if (hash1.set != hash2.set)
	{
		return false;
	}
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		if (hash1.hash[i] != hash2.hash[i])
		{
			return false;
		}
	}
	return true;
}

Hash_t initiateHash_t()
{
	Hash_t hash_t;
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		hash_t.hash[i] = '0';
	}
	hash_t.set = true;
	return hash_t;
}

Hash_t initiateDummyHash_t()
{
	Hash_t hash_t;
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		hash_t.hash[i] = '0';
	}
	hash_t.set = false;
	return hash_t;
}

bool signText_t(std::string text, EC_key privateKey, unsigned char sign[SIGN_LEN])
{
	unsigned char hash[SHA256_DIGEST_LENGTH];

	if (!SHA256((const unsigned char *)text.c_str(), text.size(), hash))
	{
		if (DEBUG_TEE)
		{
			TEE_Print("SHA256 failed");
		}
		return false;
	}
	unsigned int signLen = ECDSA_size(privateKey);
	unsigned char *buf = NULL;
	if ((buf = (unsigned char *)OPENSSL_malloc(signLen)) == NULL)
	{
		if (DEBUG_TEE)
		{
			TEE_Print("Malloc failed");
		}
		return false;
	}
	for (int k = 0; k < SHA256_DIGEST_LENGTH; k++)
	{
		buf[k] = '0';
	}

	if (!ECDSA_sign(NID_sha256, hash, SHA256_DIGEST_LENGTH, buf, &signLen, privateKey))
	{
		if (DEBUG_TEE)
		{
			TEE_Print("ECDSA_sign failed");
		}
		return false;
	}

	for (int k = 0; k < SHA256_DIGEST_LENGTH; k++)
	{
		sign[k] = buf[k];
	}
	OPENSSL_free(buf);

	return true;
}

Sign_t sign_t(Key privateKey, ReplicaID signer, std::string text)
{
	unsigned char str[SIGN_LEN];
	Sign_t sign;
	sign.signer = signer;
	sign.set = signText_t(text, privateKey, str);
	std::copy(str, str + SIGN_LEN, std::begin(sign.signtext));
	return sign;
}

Sign_t signData_t(std::string text)
{
	Sign_t sign = sign_t(privateKey, replicaId, text);
	return sign;
}

bool verifyText_t(std::string text, EC_key publicKey, unsigned char sign[SIGN_LEN])
{
	if (DEBUG_TEE)
	{
		TEE_Print("Verifying text");
	}

	unsigned int signLen = SIGN_LEN;
	unsigned char hash[SHA256_DIGEST_LENGTH];
	if (!SHA256((const unsigned char *)text.c_str(), text.size(), hash))
	{
		if (DEBUG_TEE)
		{
			TEE_Print("SHA256 failed");
		}
		return false;
	}

	bool b = ECDSA_verify(NID_sha256, hash, SHA256_DIGEST_LENGTH, sign, signLen, publicKey);

	return b;
}

bool verifySign_t(Sign_t sign_t, Key publicKey, std::string text)
{
	unsigned char *signtext = &sign_t.signtext[0];
	bool b = verifyText_t(text, publicKey, signtext);
	return b;
}

bool verifySigns_t(Signs_t signs, ReplicaID replicaId, std::map<ReplicaID, Key> publicKeys, std::string text)
{
	for (int i = 0; i < signs.size; i++)
	{
		Sign_t sign_t = signs.signs[i];
		ReplicaID replicaId = sign_t.signer;
		std::map<ReplicaID, Key>::iterator it = publicKeys.find(replicaId);
		if (it != publicKeys.end())
		{
			if (verifySign_t(sign_t, it->second, text))
			{
				if (DEBUG_TEE)
				{
					TEE_Print(("Successfully verified signature from " + std::to_string(replicaId)).c_str());
				}
			}
			else
			{
				if (DEBUG_TEE)
				{
					TEE_Print(("Failed to verify signature from " + std::to_string(replicaId)).c_str());
				}
				return false;
			}
		}
		else
		{
			if (DEBUG_TEE)
			{
				TEE_Print(("Couldn't get information for " + std::to_string(replicaId)).c_str());
			}
			return false;
		}
	}
	return true;
}

bool verify_t(Signs_t signs_t, std::string text)
{
	bool b = verifySigns_t(signs_t, replicaId, publicKeys, text);
	return b;
}

bool verifyJustification_t(Justification_t *justification_t)
{
	return verify_t(justification_t->signs, roundData2string_t(justification_t->roundData));
}

bool verifyProposalCommon_t(ProposalCommon_t *proposalCommon_t, Signs_t *signs_t)
{
	return verify_t(*signs_t, proposalCommon2string_t(*proposalCommon_t));
}

bool verifyProposalFast_t(ProposalFast_t *proposalFast_t, Signs_t *signs_t)
{
	return verify_t(*signs_t, proposalFast2string_t(*proposalFast_t));
}