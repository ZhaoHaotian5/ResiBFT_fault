#include "Checkpoint.h"

void Checkpoint::serialize(salticidae::DataStream &data) const
{
	data << this->verifyHash << this->verifyView << this->validations << this->signs;
}

void Checkpoint::unserialize(salticidae::DataStream &data)
{
	data >> this->verifyHash >> this->verifyView >> this->validations >> this->signs;
}

Checkpoint::Checkpoint()
{
	this->verifyHash = Hash();
	this->verifyView = 0;
	this->validations = Validations();
	this->signs = Signs();
}

Checkpoint::Checkpoint(Hash verifyHash, View verifyView)
{
	this->verifyHash = verifyHash;
	this->verifyView = verifyView;
	this->validations = Validations();
	this->signs = Signs();
}

Checkpoint::Checkpoint(Hash verifyHash, View verifyView, Validations validations, Sign sign)
{
	this->verifyHash = verifyHash;
	this->verifyView = verifyView;
	this->validations = validations;
	this->signs = Signs(sign);
}

Checkpoint::Checkpoint(Hash verifyHash, View verifyView, Validations validations, Signs signs)
{
	this->verifyHash = verifyHash;
	this->verifyView = verifyView;
	this->validations = validations;
	this->signs = signs;
}

Hash Checkpoint::getVerifyHash()
{
	return this->verifyHash;
}

View Checkpoint::getVerifyView()
{
	return this->verifyView;
}

Validations Checkpoint::getValidations()
{
	return this->validations;
}

Signs Checkpoint::getSigns()
{
	return this->signs;
}

std::string Checkpoint::toPrint()
{
	std::string text = "";
	text += "CHECKPOINT[";
	text += this->verifyHash.toPrint();
	text += ",";
	text += std::to_string(this->verifyView);
	text += ",";
	text += this->validations.toPrint();
	text += ",";
	text += this->signs.toPrint();
	text += "]";
	return text;
}

std::string Checkpoint::toString()
{
	std::string text = "";
	text += this->verifyHash.toString();
	text += std::to_string(this->verifyView);
	text += this->validations.toString();
	text += this->signs.toString();
	return text;
}
