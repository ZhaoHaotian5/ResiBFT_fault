#ifndef STATISTICS_H
#define STATISTICS_H

#include <chrono>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "types.h"

using Time = std::chrono::time_point<std::chrono::steady_clock>;

struct Times
{
	unsigned int num;
	double total;
	Times()
	{
		num = 0;
		total = 0.0;
	}
	Times(unsigned int num, double total) : num(num), total(total) {}
};

class Statistics
{
private:
	ReplicaID replicaId;

	std::map<View, std::multiset<double>> handleTimes;
	std::map<View, double> allHandleTimes;
	double totalHandleTime = 0.0; // Total time of spending on handling messages

	std::map<View, double> allViewTimes;
	double totalViewTime = 0.0; // Total time of spending on views

	std::set<View> common2fastView;
	std::set<View> fast2commonView;

	unsigned int executeViews = 0;

public:
	Statistics();

	void setReplicaId(ReplicaID replicaId);
	ReplicaID getReplicaId();

	double getTotalHandleTimes();
	unsigned int getTotalHandleNum();
	void addHandleTimes(View view, double value);
	void addAllHandleTimes(View view);

	double getTotalViewTimes();
	unsigned int getTotalViewNum();
	void addAllViewTimes(View view, double value);

	void addCommon2fast(View view);
	unsigned int getCommon2fast();

	void addFast2common(View view);
	unsigned int getFast2common();

	double getTotalViewTimesTentative();

	void increaseExecuteViews();

	std::string toString();

	friend std::ostream &operator<<(std::ostream &os, const Statistics &data);
};

#endif
