#ifndef VALIDATION_H
#define VALIDATION_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "salticidae/stream.h"

class Validation
{
private:
	bool set; // True if [Validation] is not dummy
	bool verifier; // True if [Validation] is accepted

public:
	void serialize(salticidae::DataStream &data) const;
	void unserialize(salticidae::DataStream &data);

	Validation();
	Validation(bool b);
	Validation(bool b1, bool b2);

	bool isSet();
	bool isAccepted();

	std::string toPrint();
	std::string toString();

	bool operator==(const Validation &data) const;
};

#endif
