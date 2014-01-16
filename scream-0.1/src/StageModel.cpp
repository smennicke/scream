/*
 * StageModel.cc
 *
 *  Created on: Sep 22, 2013
 *      Author: stephan
 */

#include <fstream>
#include <iostream>
#include <sstream>

#include "cmdline.h"
#include "Feature.h"
#include "Stage.h"
#include "StageModel.h"

#include "verbose.h"

using namespace std;

extern gengetopt_args_info args_info;

StageModel::StageModel(const vector<Feature *> &config, const Strategy &strat,
		const InnerStrategy &istrat, Feature *root) :
		configuration_(config), strat_(strat), istrat_(istrat), stageNet_(NULL), root_(root)
{
//	stages_.push_back(new Stage());
//	isSound_ = false;

	setup();
}

StageModel::~StageModel()
{
	if (stageNet_ != NULL)
		delete stageNet_;
	stages_.clear();
}

void StageModel::setup()
{
	triedOnes_ = 0;
	soundOnes_ = 0;
	set<string> sequences;
	do
	{
		stringstream note;
		note << "Try sequence " << triedOnes_ + 1 << ": "
				<< getProductSequence();
		scream_status(note.str());
//		cin.ignore();
		if (sequences.count(getProductSequence()))
		{
			scream_error(
					"Do not trust the C++ algorithms library (next permutation)");
			exit(EXIT_FAILURE);
		}
		sequences.insert(getProductSequence());
		for (int i = 0; i < stages_.size(); ++i)
			delete stages_[i];
		stages_.clear();
		hashMap_.clear();
		for (int i = 0; i < configuration_.size(); ++i)
		{
			addFeature(configuration_[i]);
			stringstream stat;
			stat << "Progress: " << i + 1 << " / " << configuration_.size();
			scream_status(stat.str());
			isSound_ = false;

			computeRecursive();

			if (isSound_)
			{
				scream_status("Found sound Sub-Stagemodel: " + toString());
//				return;
			}
			else
			{
				scream_status("Did not find a sound Sub-Stagemodel");
				break;
			}
		}

		triedOnes_++;
	} while (/* option: --try-all */((!isSound() && args_info.try_all_flag)
			|| (args_info.all_flag))
			&& next_permutation(configuration_.begin(), configuration_.end()));
}

void StageModel::addFeature(Feature *f)
{
	switch (strat_)
	{
	case BACKWARDS: // in this strategy, each feature gets a new stage
					// backtracking means that the fist feature of the last stage
					// is added to the stage before the last
//		if (stages_.back()->size() != 0)
		stages_.push_back(new Stage());
		/* no break */
	case FORWARDS: // in this strategy, new stages are only created when
				   // necessary
	default:
		if (stages_.size() == 0)
			stages_.push_back(new Stage());
		stages_.back()->push_back(f);
		break;
	}
}

PetriNet &StageModel::buildNet()
{
	vector<string> &toBeDeleted = *(new vector<string>());
	set<Feature *> added;

	if (stageNet_ != NULL)
		delete stageNet_;
	stageNet_ = new PetriNet();

	// build staged net and check for soundness
	for (int j = 0; j < stages_.size(); ++j)
	{
//		if (stages_[j]->size() == 0)
//			continue;
		PetriNet &nextStage = stages_[j]->getWorkflow(j, added);
		for (int i = 0; i < stages_[j]->size(); ++i)
			added.insert(stages_[j]->getFeatures()[i]);
		wfmComposition(*stageNet_, nextStage, toBeDeleted, j);
	}

	scream_status("Delete root feature's deselection operation");
	Transition *rootRem = stageNet_->findTransition(root_->getRemoveFeature().getName());
	if (!rootRem) {
		scream_error("Remove root transition not found!");
	}
	set<Node *> places = rootRem->getPreset();
	PNAPI_FOREACH(p, places) {
		if ((*p)->getPostset().size() < 2) { /* if after deletion of rootRem, there is no postset place */
			stageNet_->deletePlace(*static_cast<Place *>(*p));
		}
	}
	places = rootRem->getPostset();
	PNAPI_FOREACH(p, places) {
		if ((*p)->getPreset().size() < 2) {
			stageNet_->deletePlace(*static_cast<Place *>(*p));
		}
	}
	stageNet_->deleteTransition(*rootRem);
	scream_status("Finished deleting root feature's deselection operation");

	// Clean up
	delete &toBeDeleted;

	return *stageNet_;
}

void StageModel::computeRecursive()
{
	PetriNet &temp = buildNet();
	if (hashMap_.count(toString()))
	{
		return;
	}
	hashMap_.insert(toString());
	if (!(hashMap_.size() % 50))
		std::cerr << "#SMs visited: " << hashMap_.size() << std::endl;
	scream_status("Try " + toString());

	isSound_ = false;
	if (hasNoDeadTransitions(*stageNet_)
			&& isLazySound(temp, stages_[stages_.size() - 1]->getOutput()))
	{
		isSound_ = true;
//		if (args_info.all_flag || args_info.all_given_sequence_flag)
//		{
//			soundOnes_++;
//			scream_resultfile(this);
//		}
//		else
		return;
	}

	// Change Stage Model iteratively
	switch (istrat_)
	{
	case FIRST:
		for (int i = 0; i < stages_.size(); ++i)
		{
			if ((strat_ == FORWARDS && stages_[i]->size() == 1)
					|| (strat_ == BACKWARDS && i == 0))
				continue;

			// Shift and Recursive Call
			shift(i);
			computeRecursive();

			// is sound?
			if (isSound_)
				return;
			shiftBack(i);
		}
		break;
	case LAST:
		for (int i = stages_.size() - 1; i >= 0; --i)
		{
			if ((strat_ == FORWARDS && stages_[i]->size() == 1)
					|| (strat_ == BACKWARDS && i == 0))
				continue;

			// Shift and Recursive Call
			string goal = toString();
			shift(i);
			computeRecursive();

			// is sound?
			if (isSound_)
				return;

			// shifting back
//			std::cout << "from: " << toString() << endl;
//			std::cout << "goal: " << goal << endl;
			shiftBack(i);
//			std::cout << "real: " << toString() << endl;
			if (goal != toString())
			{
				if (strat_ == FORWARDS) {
					scream_error("Too much of a fix");
					exit(EXIT_FAILURE);
				}
				for (int j = stages_.size() - 1; j > i; --j)
					while (stages_[j]->size())
						shiftRight(j);
				while (stages_[i]->size() - 1)
					shiftRight(i);
			}
			if (goal != toString())
			{
				scream_error("Fix does not work!");
				exit(EXIT_FAILURE);
			}
		}
		break;
	case FORELAST:
	default:
		cerr << "SM Error: No valid Strategy taken" << endl;
		break;
	}
}

void StageModel::shift(const int pos)
{
//	std::cout << "shift " << pos << " in " << toString() << std::endl;
	switch (strat_)
	{
	case FORWARDS:
		shiftRight(pos);
		break;
	case BACKWARDS:
	default:
		shiftLeft(pos);
		break;
	}
}

void StageModel::shiftBack(const int pos)
{
//	std::cout << "shift back " << pos << std::endl;
	switch (strat_)
	{
	case BACKWARDS:
		shiftRight(pos - 1);
		break;
	case FORWARDS:
	default:
		shiftLeft(pos + 1);
		break;
	}
}

void StageModel::shiftRight(const int pos)
{
	if (pos >= stages_.size())
	{
		scream_error("Try to shift right from over-maximum");
		exit(EXIT_FAILURE);
	}
	if (pos == stages_.size() - 1)
	{
		stages_.push_back(new Stage());
	}
	stages_[pos + 1]->push_front(stages_[pos]->pop_back());
}

void StageModel::shiftRight2(const int pos)
{
	if (pos >= stages_.size())
	{
		scream_error("Try to shift right from over-maximum");
		exit(EXIT_FAILURE);
	}

	stages_.push_back(new Stage());
	Stage *temp = stages_.back();
	for (int i = pos + 1; i < stages_.size(); ++i)
		stages_[i+1] = stages_[i];
	stages_[pos + 1] = temp;
	stages_[pos + 1]->push_front(stages_[pos]->pop_back());
}

void StageModel::shiftLeft(const int pos)
{
	if (pos == 0)
	{
		scream_error("Try to shift left from 0");
		exit(EXIT_FAILURE);
	}
	stages_[pos - 1]->push_back(stages_[pos]->pop_front());
	if (stages_[pos]->size() > 0)
		return;
	Stage *temp = stages_[pos];
	for (int i = pos; i < stages_.size() - 1; ++i)
	{
		stages_[i] = stages_[i + 1];
	}
	stages_.pop_back();
	if (temp != NULL)
		delete temp;
}

const string StageModel::toString() const
{
	string result;
	for (int i = 0; i < stages_.size(); ++i)
//		if (stages_[i]->size())
		result += stages_[i]->toString();
	return result;
}

const float StageModel::ratio() const
{
	return ((float) numFeatures() / (float) size());
}

const int StageModel::min() const
{
	int result = stages_[0]->size();
	for (int i = 0; i < stages_.size(); ++i)
	{
//		if (stages_[i]->size() > 0)
		result = (result > stages_[i]->size() ? stages_[i]->size() : result);
	}

	return result;
}

const int StageModel::max() const
{
	int result = stages_[0]->size();
	for (int i = 0; i < stages_.size(); ++i)
	{
		result = (result < stages_[i]->size() ? stages_[i]->size() : result);
	}

	return result;
}

const int StageModel::numFeatures() const
{
	float result = 0;
	for (int i = 0; i < stages_.size(); ++i)
	{
		result += stages_[i]->size();
	}

	return result;
}

const string StageModel::getProductSequence() const
{
	if (configuration_.empty())
		return string();
	string result = configuration_[0]->getName();
	for (int i = 1; i < configuration_.size(); ++i)
		result += " " + configuration_[i]->getName();
	return result;
}

//const int StageModel::size() const
//{
//	int result = 0;
//	for (int i = 0; i < stages_.size(); ++i)
//		result += ( stages_[i]->size() ? 1 : 0 );
//	return result;
//}

