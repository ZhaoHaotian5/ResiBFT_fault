#ifndef PROPOSALFAST_H
#define PROPOSALFAST_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "salticidae/stream.h"
#include "Accumulator.h"
#include "Block.h"
#include "Justification.h"
#include "Signs.h"

class ProposalFast
{
private:
	Accumulator accumulator;
	Block block;

public:
	void serialize(salticidae::DataStream &data) const;
	void unserialize(salticidae::DataStream &data);

	ProposalFast();
	ProposalFast(Accumulator accumulator, Block block);

	Accumulator getAccumulator();
	Block getBlock();

	std::string toPrint();
	std::string toString();
};

#endif
