#include "Validations.h"

void Validations::serialize(salticidae::DataStream &data) const
{
	data << this->size;
	for (int i = 0; i < NUM_REPLICAS; i++)
	{
		data << this->validations[i];
	}
}

void Validations::unserialize(salticidae::DataStream &data)
{
	data >> this->size;
	for (int i = 0; i < NUM_REPLICAS; i++)
	{
		data >> this->validations[i];
	}
}

Validations::Validations()
{
	this->size = 0;
	for (int i = 0; i < NUM_REPLICAS; i++)
	{
		this->validations[i] = Validation();
	}
}

Validations::Validations(Validation validation)
{
	this->size = 1;
	this->validations[0] = validation;
}

Validations::Validations(unsigned int size, Validation validations[NUM_REPLICAS])
{
	this->size = size;
	for (int i = 0; i < NUM_REPLICAS; i++)
	{
		this->validations[i] = validations[i];
	}
}

Validations::Validations(salticidae::DataStream &data)
{
	unserialize(data);
}

unsigned int Validations::getSize()
{
	return this->size;
}

Validation Validations::get(unsigned int n)
{
	return this->validations[n];
}

void Validations::add(Validation validation)
{
	this->validations[this->size] = validation;
	this->size++;
}

void Validations::addUpto(Validations validations, unsigned int n)
{
	for (int i = 0; i < validations.getSize() && this->size < n; i++)
	{
		this->add(validations.get(i));
	}
}

bool Validations::isAccepted()
{
	if (this->size == 0)
	{
		return false;
	}

	unsigned int accepted = 0;
	unsigned int threshold = ceil((this->size + 1) / 2);

	for (unsigned int i = 0; i < this->size; i++)
	{
		if (this->validations[i].isSet() && this->validations[i].isAccepted())
		{
			accepted++;
		}
	}

	if (accepted >= threshold)
	{
		return true;
	}
	else
	{
		return false;
	}
}

std::string Validations::toPrint()
{
	std::string textValidations = "";
	for (int i = 0; i < this->size; i++)
	{
		textValidations += this->validations[i].toPrint();
	}
	std::string text = "";
	text += "VALIDATIONS[";
	text += std::to_string(this->size);
	text += "-";
	text += textValidations;
	text += "]";
	return text;
}

std::string Validations::toString()
{
	std::string text = "";
	text += std::to_string(this->size);
	for (int i = 0; i < this->size; i++)
	{
		text += this->validations[i].toString();
	}
	return text;
}

bool Validations::operator==(const Validations &data) const
{
	if (this->size != data.size)
	{
		return false;
	}
	for (int i = 0; i < NUM_REPLICAS && i < this->size; i++)
	{
		if (!(this->validations[i] == data.validations[i]))
		{
			return false;
		}
	}
	return true;
}