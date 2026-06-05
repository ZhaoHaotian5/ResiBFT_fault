#include "Certificate.h"

void Certificate::serialize(salticidae::DataStream &data) const
{
	data << this->block << this->validations << this->signs;
}

void Certificate::unserialize(salticidae::DataStream &data)
{
	data >> this->block >> this->validations >> this->signs;
}

Certificate::Certificate()
{
	this->block = Block();
	this->validations = Validations();
	this->signs = Signs();
}

Certificate::Certificate(Block block, Validations validations, Sign sign)
{
	this->block = block;
	this->validations = validations;
	this->signs = Signs(sign);
}

Certificate::Certificate(Block block, Validations validations, Signs signs)
{
	this->block = block;
	this->validations = validations;
	this->signs = signs;
}

Block Certificate::getBlock()
{
	return this->block;
}

Validations Certificate::getValidations()
{
	return this->validations;
}

Signs Certificate::getSigns()
{
	return this->signs;
}

std::string Certificate::toPrint()
{
	std::string text = "";
	text += "CERTIFICATE[";
	text += this->block.toPrint();
	text += ",";
	text += this->validations.toPrint();
	text += ",";
	text += this->signs.toPrint();
	text += "]";
	return text;
}

std::string Certificate::toString()
{
	std::string text = "";
	text += this->block.toString();
	text += this->validations.toString();
	text += this->signs.toString();
	return text;
}
