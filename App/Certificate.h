#ifndef CERTIFICATE_H
#define CERTIFICATE_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "salticidae/stream.h"
#include "Block.h"
#include "Signs.h"
#include "Validations.h"

class Certificate
{
private:
	Block block;
	Validations validations;
	Signs signs;

public:
	void serialize(salticidae::DataStream &data) const;
	void unserialize(salticidae::DataStream &data);

	Certificate();
	Certificate(Block block, Validations validations, Sign sign);
	Certificate(Block block, Validations validations, Signs signs);

	Block getBlock();
	Validations getValidations();
	Signs getSigns();

	std::string toPrint();
	std::string toString();
};

#endif
