#ifndef COMMITTEE_H
#define COMMITTEE_H

#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "salticidae/stream.h"
#include "config.h"
#include "types.h"

class Committee
{
private:
	bool set;
	unsigned int size;
	ReplicaID committee[NUM_COMMITTEE_MEMBERS];

public:
	void serialize(salticidae::DataStream &data) const;
	void unserialize(salticidae::DataStream &data);

	Committee();
	Committee(bool b);
	Committee(std::set<ReplicaID> committee);

	bool isSet();
	unsigned int getSize();
	ReplicaID *getCommittee();

	bool find(ReplicaID replicaID);
	bool isEmpty();

	std::string toPrint();
	std::string toString();
};

#endif
