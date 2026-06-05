#include "ProposalFast.h"

void ProposalFast::serialize(salticidae::DataStream &data) const
{
	data << this->accumulator << this->block;
}

void ProposalFast::unserialize(salticidae::DataStream &data)
{
	data >> this->accumulator >> this->block;
}

ProposalFast::ProposalFast() {}

ProposalFast::ProposalFast(Accumulator accumulator, Block block)
{
	this->accumulator = accumulator;
	this->block = block;
}

Accumulator ProposalFast::getAccumulator()
{
	return this->accumulator;
}

Block ProposalFast::getBlock()
{
	return this->block;
}

std::string ProposalFast::toPrint()
{
	std::string text = "";
	text += "PROPOSALFAST[";
	text += this->accumulator.toPrint();
	text += ",";
	text += this->block.toPrint();
	text += "]";
	return text;
}

std::string ProposalFast::toString()
{
	std::string text = "";
	text += this->accumulator.toString();
	text += this->block.toString();
	return text;
}