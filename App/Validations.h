#ifndef VALIDATIONS_H
#define VALIDATIONS_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "salticidae/stream.h"
#include "parameters.h"
#include "Validation.h"

class Validations
{
private:
	unsigned int size;
	Validation validations[NUM_REPLICAS];

public:
	void serialize(salticidae::DataStream &data) const;
	void unserialize(salticidae::DataStream &data);

	Validations();
	Validations(Validation validation);
	Validations(unsigned int size, Validation validations[NUM_REPLICAS]);
	Validations(salticidae::DataStream &data);

	unsigned int getSize();
	Validation get(unsigned int n);

	void add(Validation validation);
	void addUpto(Validations validations, unsigned int n);
	bool isAccepted();

	std::string toPrint();
	std::string toString();

	bool operator==(const Validations &data) const;
};

#endif
