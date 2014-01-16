/*
 * FeatureModel.cc
 *
 *  Created on: Sep 22, 2013
 *      Author: stephan
 */

#include <iostream>
#include <sstream>

#include "Feature.h"
#include "FeatureGroup.h"
#include "FeatureModel.h"
#include "StageModel.h"

#include "tools.h"

using namespace std;

FeatureModel::FeatureModel(Feature *root) :
		root_(root)
{
	features_.push_back(root_);
	root_->setType(MANDATORY);

	net_ = NULL;
}

FeatureModel::~FeatureModel()
{
	if (root_)
		delete root_;
	if (net_)
		delete net_;

	root_ = NULL;
	net_ = NULL;
}

void FeatureModel::setRootFeature(Feature &root)
{
	root_ = &root;
}

Feature &FeatureModel::getRootFeature() const
{
	return *root_;
}

Feature &FeatureModel::addFeature(const FeatureType &type, const string &name,
		Feature *parent)
{
	features_.push_back(new Feature(type, name, parent));
	return *(features_.back());
}

FeatureGroup &FeatureModel::addGroup(const GroupType &type, Feature *parent)
{
	return *(new FeatureGroup(type, parent, this));
}

Feature &FeatureModel::addGroupFeature(FeatureGroup &group, const string &name)
{
	features_.push_back(new Feature(IN_GROUP, name, NULL, &group));
	return *features_.back();
}

PetriNet &FeatureModel::getPetriNet()
{
	if (net_ == NULL)
		buildPetriNet();
	return *net_;
}

void FeatureModel::buildPetriNet()
{
	net_ = new PetriNet();
	for (int i = 0; i < features_.size(); ++i)
		synchronousProduct(*net_, features_[i]->getPattern());
}

bool FeatureModel::checkConfiguration(const string &conf) const
{
	if (conf.empty())
		return true;

	vector<Feature *> config;
	istringstream ss(conf);

	while (ss)
	{
		string sub;
		ss >> sub;
		if (!sub.empty())
			for (int i = 0; i < features_.size(); ++i)
			{
				if (sub == features_[i]->getName())
				{
					config.push_back(features_[i]);
					break;
				}
			}
	}

	if (features_.size() != config.size())
	{
		scream_error(
				"Configuration (Feature) sequence is not consistent with the feature model");
		scream_error(conf + " given");
		stringstream expected;
		for (int i = 0; i < features_.size(); ++i)
		{
			expected << " " << features_[i]->getName();
		}
		scream_error(expected.str() + " expected");

		return false;
	}

	return true;
}

/*
 * Input:
 * 	- conf : string .. a white space separated list of features
 * 	- strat : Strategy .. represents the evaluation strategy
 */
StageModel &FeatureModel::computeStageModel(vector<Feature *> config,
		const Strategy &strat, const InnerStrategy &istrat)
{
	if (config.empty())
	{
		config = features_;
	}

//	scream_status("Delete root feature's deselection operation");
//	set<Node *> places = root_->getRemoveFeature().getPreset();
//	PNAPI_FOREACH(p, places) {
//		if (*p != &root_->getInputFeature()) {
//			root_->getPattern().deletePlace(*static_cast<Place *>(*p));
//		}
//	}
//	places = root_->getRemoveFeature().getPostset();
//	PNAPI_FOREACH(p, places) {
//		if (*p != &root_->getOutputFeature()) {
//			root_->getPattern().deletePlace(*static_cast<Place *>(*p));
//		}
//	}
//	root_->getPattern().deleteTransition(root_->getRemoveFeature());
//	scream_status("Finished deleting root feature's deselection operation");

	return *new StageModel(config, strat, istrat, root_);
}

Feature *FeatureModel::findFeature(const string &name)
{
	Feature *result = NULL;
	for (int i = 0; i < features_.size(); ++i)
		if (features_[i]->getName() == name)
		{
			result = features_[i];
			break;
		}
	return result;
}

const float FeatureModel::ctcr() const
{	return ((float) ctcs_ / (float) size()) * 100.0;
}

