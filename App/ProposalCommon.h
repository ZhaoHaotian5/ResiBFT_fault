#ifndef PROPOSALCOMMON_H
#define PROPOSALCOMMON_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "salticidae/stream.h"
#include "Accumulator.h"
#include "Block.h"
#include "Justification.h"
#include "Signs.h"

class ProposalCommon
{
private:
	Justification justification;
	Block block;

public:
	void serialize(salticidae::DataStream &data) const;
	void unserialize(salticidae::DataStream &data);

	ProposalCommon();
	ProposalCommon(Justification justification, Block block);

	Justification getJustification();
	Block getBlock();

	std::string toPrint();
	std::string toString();
};

#endif
