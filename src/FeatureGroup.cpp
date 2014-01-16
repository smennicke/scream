/*
 * FeatureGroup.cpp
 *
 *  Created on: Sep 23, 2013
 *      Author: stephan
 */

#include "Feature.h"
#include "FeatureGroup.h"
#include "FeatureModel.h"

#include <sstream>

using std::stringstream;

FeatureGroup::FeatureGroup(const GroupType &type, Feature *parent,
		FeatureModel *fm) :
		type_(type), fm_(fm)
{
	if (parent != NULL)
	{
		setParent(*parent);
	}
	else
	{
		parent_ = parent;
	}
}

FeatureGroup::~FeatureGroup()
{
//	for (int i = 0; i < features_.size(); ++i)
//		if (features_[i])
//		{
//			delete features_[i];
//			features_[i] = NULL;
//		}
}

void FeatureGroup::setType(const GroupType &type)
{
	type_ = type;
}

const GroupType &FeatureGroup::getType() const
{
	return type_;
}

void FeatureGroup::addFeature(Feature &feature)
{

	// Establish Parent Child Relationship
	if (parent_ != NULL)
	{
		feature.setParent(*parent_);
	}

	// Add Group Relation
	PetriNet *fnet = &feature.getPattern();
	Place *ptc = &fnet->createPlace(
			"p_" + feature.getRemoveFeature().getName()); // Parent To Child
	fnet->createArc(*ptc, feature.getRemoveFeature());

	if (parent_ != NULL)
	{
		if (parent_ != &fm_->getRootFeature())
		{
			Place *ptc3 = &parent_->getPattern().createPlace(ptc->getName());
			parent_->getPattern().createArc(parent_->getRemoveFeature(), *ptc3);
		}

		Place *ctp = &fnet->createPlace(parentAddFeature_->getName()); // Child To Parent
		fnet->createArc(feature.getAddFeature(), *ctp);
	}

	for (int i = 0; i < features_.size(); ++i)
	{
		if (parent_ != &fm_->getRootFeature())
		{
			Place *ptc1 = &fnet->createPlace(
					"p_" + features_[i]->getRemoveFeature().getName());
			fnet->createArc(feature.getAddFeature(), *ptc1);

			Place *ptc2 = &features_[i]->getPattern().createPlace(
					ptc->getName());
			features_[i]->getPattern().createArc(features_[i]->getAddFeature(),
					*ptc2);
		}
	}

	if (type_ == XOR)
	{
		for (int i = 0; i < features_.size(); ++i)
		{
			features_[i]->excludes(feature);
		}
	}

	features_.push_back(&feature);
}

void FeatureGroup::setParent(Feature &parent)
{
	vector<Feature *> old = features_;
	features_.clear();

	parent_ = &parent;
	stringstream pname;
	pname << "p_" << parent_->getAddFeature().getName() << "_" << this;
	parentAddFeature_ = &parent_->getPattern().createPlace(pname.str());
	parent_->getPattern().createArc(*parentAddFeature_,
			parent_->getAddFeature());

	for (int i = 0; i < old.size(); ++i)
	{
		addFeature(*old[i]);
	}
}

