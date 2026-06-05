#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "salticidae/stream.h"
#include "Hash.h"
#include "Signs.h"
#include "Validations.h"

class Checkpoint
{
private:
	Hash verifyHash;
	View verifyView;
	Validations validations;
	Signs signs;

public:
	void serialize(salticidae::DataStream &data) const;
	void unserialize(salticidae::DataStream &data);

	Checkpoint();
	Checkpoint(Hash verifyHash, View verifyView);
	Checkpoint(Hash verifyHash, View verifyView, Validations validations, Sign sign);
	Checkpoint(Hash verifyHash, View verifyView, Validations validations, Signs signs);

	Hash getVerifyHash();
	View getVerifyView();
	Validations getValidations();
	Signs getSigns();

	std::string toPrint();
	std::string toString();
};

#endif
