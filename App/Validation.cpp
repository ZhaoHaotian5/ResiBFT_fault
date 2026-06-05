#include "Validation.h"

void Validation::serialize(salticidae::DataStream &data) const
{
	data << this->set << this->verifier;
}

void Validation::unserialize(salticidae::DataStream &data)
{
	data >> this->set >> this->verifier;
}

Validation::Validation()
{
	this->set = true;
	this->verifier = true;
}

Validation::Validation(bool b)
{
	this->set = b;
	this->verifier = true;
}

Validation::Validation(bool b1, bool b2)
{
	this->set = b1;
	this->verifier = b2;
}

bool Validation::isSet()
{
	return this->set;
}

bool Validation::isAccepted()
{
	return this->verifier;
}

std::string Validation::toPrint()
{
	std::string textValidation;
	if (this->set)
	{
		if (this->verifier)
		{
			textValidation = "Accepted";
		}
		else
		{
			textValidation = "Rejected";
		}
	}
	else
	{
		textValidation = "Timeout";
	}
	std::string text = "VALIDATION[";
	text += textValidation;
	text += "]";
	return text;
}

std::string Validation::toString()
{
	std::string text = "";
	text += std::to_string(this->set);
	text += std::to_string(this->verifier);
	return text;
}

bool Validation::operator==(const Validation &data) const
{
	if (this->set != data.set || this->verifier != data.verifier)
	{
		return false;
	}
	else
	{
		return true;
	}
}
