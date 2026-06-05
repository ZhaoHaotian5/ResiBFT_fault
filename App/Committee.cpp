#include "Committee.h"

void Committee::serialize(salticidae::DataStream &data) const
{
	data << this->set << this->size;
	for (int i = 0; i < this->size; i++)
	{
		data << this->committee[i];
	}
}

void Committee::unserialize(salticidae::DataStream &data)
{
	data >> this->set >> this->size;
	for (int i = 0; i < this->size; i++)
	{
		data >> this->committee[i];
	}
}

Committee::Committee()
{
	this->set = false;
	this->size = 0;
	for (int i = 0; i < NUM_COMMITTEE_MEMBERS; i++)
	{
		this->committee[i] = 0;
	}
}

Committee::Committee(bool b)
{
	this->set = b;
	this->size = 0;
	for (int i = 0; i < NUM_COMMITTEE_MEMBERS; i++)
	{
		this->committee[i] = 0;
	}
}

Committee::Committee(std::set<ReplicaID> committee)
{
	this->set = true;
	this->size = committee.size();
	int i = 0;
	for (std::set<ReplicaID>::iterator itCommittee = committee.begin(); itCommittee != committee.end(); itCommittee++)
	{
		ReplicaID member = *itCommittee;
		this->committee[i++] = member;
	}
}

bool Committee::isSet()
{
	return this->set;
}

unsigned int Committee::getSize()
{
	return this->size;
}

ReplicaID *Committee::getCommittee()
{
	return this->committee;
}

bool Committee::find(ReplicaID replicaID)
{
	if (this->size)
	{
		for (int i = 0; i < NUM_COMMITTEE_MEMBERS; i++)
		{
			if (this->committee[i] == replicaID)
			{
				return true;
			}
		}
	}
	return false;
}

bool Committee::isEmpty()
{
	if (this->set && this->size == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

std::string Committee::toPrint()
{
	std::string textCommittee = "";
	for (int i = 0; i < this->size; i++)
	{
		textCommittee += std::to_string(this->committee[i]);
		if (i != this->size - 1)
		{
			textCommittee += ",";
		}
	}
	std::string text = "";
	text += "COMMITTEE[";
	text += std::to_string(this->set);
	text += "-";
	text += std::to_string(this->size);
	text += "-";
	text += textCommittee;
	text += "]";
	return text;
}

std::string Committee::toString()
{
	std::string text = "";
	text += std::to_string(this->set);
	text += std::to_string(this->size);
	for (int i = 0; i < this->size; i++)
	{
		text += std::to_string(this->committee[i]);
	}
	return text;
}
