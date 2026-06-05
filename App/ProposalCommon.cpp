#include "ProposalCommon.h"

void ProposalCommon::serialize(salticidae::DataStream &data) const
{
	data << this->justification << this->block;
}

void ProposalCommon::unserialize(salticidae::DataStream &data)
{
	data >> this->justification >> this->block;
}

ProposalCommon::ProposalCommon() {}

ProposalCommon::ProposalCommon(Justification justification, Block block)
{
	this->justification = justification;
	this->block = block;
}

Justification ProposalCommon::getJustification()
{
	return this->justification;
}

Block ProposalCommon::getBlock()
{
	return this->block;
}

std::string ProposalCommon::toPrint()
{
	std::string text = "";
	text += "PROPOSALCOMMON[";
	text += this->justification.toPrint();
	text += ",";
	text += this->block.toPrint();
	text += "]";
	return text;
}

std::string ProposalCommon::toString()
{
	std::string text = "";
	text += this->justification.toString();
	text += this->block.toString();
	return text;
}