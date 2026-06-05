#include "Statistics.h"

Statistics::Statistics()
{
	this->replicaId = 0;
	this->totalHandleTime = 0.0;
	this->totalViewTime = 0.0;
}

void Statistics::setReplicaId(ReplicaID replicaId)
{
	this->replicaId = replicaId;
}

ReplicaID Statistics::getReplicaId()
{
	return this->replicaId;
}

double Statistics::getTotalHandleTimes()
{
	double totalHandleTimes = 0;
	for (std::map<View, double>::iterator it = this->allHandleTimes.begin(); it != this->allHandleTimes.end(); it++)
	{
		totalHandleTimes += it->second;
	}
	return totalHandleTimes;
}

unsigned int Statistics::getTotalHandleNum()
{
	unsigned int totalHandleNum = 0;
	for (std::map<View, std::multiset<double>>::iterator it = this->handleTimes.begin(); it != this->handleTimes.end(); it++)
	{
		totalHandleNum += it->second.size();
	}
	return totalHandleNum;
}

void Statistics::addHandleTimes(View view, double value)
{
	this->totalHandleTime += value;
	this->handleTimes[view].insert(value);
}

void Statistics::addAllHandleTimes(View view)
{
	std::multiset<double> handleTime = this->handleTimes[view];
	double totalHandleTimes = 0;
	for (std::multiset<double>::iterator it = handleTime.begin(); it != handleTime.end(); it++)
	{
		totalHandleTimes += *it;
	}
	this->allHandleTimes[view] = totalHandleTimes;
}

double Statistics::getTotalViewTimes()
{
	double totalViewTimes = 0;
	for (std::map<View, double>::iterator it = this->allViewTimes.begin(); it != this->allViewTimes.end(); it++)
	{
		totalViewTimes += it->second;
	}
	return totalViewTimes;
}

unsigned int Statistics::getTotalViewNum()
{
	return this->allViewTimes.size();
}

void Statistics::addAllViewTimes(View view, double value)
{
	this->totalViewTime += value;
	this->allViewTimes[view] = value;
}

void Statistics::addCommon2fast(View view)
{
	this->common2fastView.insert(view);
}

unsigned int Statistics::getCommon2fast()
{
	return this->common2fastView.size();
}

void Statistics::addFast2common(View view)
{
	this->fast2commonView.insert(view);
}

unsigned int Statistics::getFast2common()
{
	return this->fast2commonView.size();
}

double Statistics::getTotalViewTimesTentative()
{
	double totalViewTentative = 0.0;

	std::set<View> fastIntervalViews;
	for (std::set<View>::iterator itStart = this->common2fastView.begin(); itStart != this->common2fastView.end(); itStart++)
	{
		View common2fastView = *itStart;
		std::set<View>::iterator itEnd = this->fast2commonView.upper_bound(common2fastView);

		if (itEnd == this->fast2commonView.end())
		{
			for (View view = common2fastView + 1; view <= this->executeViews; view++)
			{
				fastIntervalViews.insert(view);
				if (view + 1 <= this->executeViews)
				{
					totalViewTentative += this->allViewTimes[view];
					totalViewTentative += this->allViewTimes[view + 1];
				}
				else
				{
					totalViewTentative += this->allViewTimes[view];
				}
			}
			break;
		}

		View fast2commonView = *itEnd;
		for (View view = common2fastView + 1; view < fast2commonView; view++)
		{
			fastIntervalViews.insert(view);
			if (view + 1 != fast2commonView)
			{
				totalViewTentative += this->allViewTimes[view];
				totalViewTentative += this->allViewTimes[view + 1];
			}
		}
	}

	for (std::map<View, double>::iterator it = this->allViewTimes.begin(); it != this->allViewTimes.end(); it++)
	{
		View view = it->first;
		if (fastIntervalViews.find(view) == fastIntervalViews.end())
		{
			totalViewTentative += it->second;
		}
	}

	return totalViewTentative;
}

void Statistics::increaseExecuteViews()
{
	this->executeViews++;
}

std::string Statistics::toString()
{
	std::ostringstream os;
	os << "STATISTICS"
	   << "[replicaId = " << this->replicaId
	   << ", executeViews = " << this->executeViews
	   << ", totalHandleTime = " << this->totalHandleTime / this->executeViews
	   << ", totalViewTime = " << this->totalViewTime / this->executeViews
	   << "]";
	return os.str();
}

std::ostream &operator<<(std::ostream &os, const Statistics &data)
{
	os << std::to_string(data.replicaId);
	return os;
}
