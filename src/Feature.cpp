/*
 * Feature.cpp
 *
 *  Created on: Sep 22, 2013
 *      Author: stephan
 */

#include "Feature.h"
#include "FeatureGroup.h"
#include "FeatureModel.h"

#include "tools.h"

extern int fcount;

Feature::Feature(const FeatureType &type, const string &name, Feature *parent,
		FeatureGroup *group) :
		type_(type), name_(name), parent_(NULL), netPattern_(NULL)
{
	scream_status("Created feature '" + name_ + "'");
	fcount++;
	// Create Basic Petri net Pattern
	netPattern_ = new PetriNet();
	addFeature_ = &netPattern_->createTransition("+" + name_);
	removeFeature_ = &netPattern_->createTransition("-" + name_);
	inputFeature_ = &netPattern_->createPlace("in_" + name_);
	outputFeature_ = &netPattern_->createPlace("out_" + name_);

	netPattern_->createArc(*inputFeature_, *addFeature_);
	netPattern_->createArc(*inputFeature_, *removeFeature_);
	netPattern_->createArc(*addFeature_, *outputFeature_);
	netPattern_->createArc(*removeFeature_, *outputFeature_);

	if (parent != NULL)
	{
		scream_status("with parent '" + parent->getName() + "'");
		setParent(*parent);
	}
	if (group != NULL)
		setGroup(*group);
}

Feature::~Feature()
{
//	if (parent_ != NULL)
//		delete parent_;
	for (int i = 0; i < children_.size(); ++i)
		if (children_[i])
		{
			delete children_[i];
			children_[i] = NULL;
		}
	if (netPattern_)
	{
		delete netPattern_;
		netPattern_ = NULL;
	}
//	if (group_ != NULL)
//	{
//		delete group_;
//		group_ = NULL;
//	}

//	addFeature_ = NULL;
//	removeFeature_ = NULL;
//	inputFeature_ = NULL;
//	outputFeature_ = NULL;
}

//FeatureModel &Feature::getFeatureModel() const {
//	return *fm_;
//}

void Feature::setType(const FeatureType &type)
{
	type_ = type;
}

void Feature::setName(const string &name)
{
	name_ = name;
}

const string &Feature::getName() const
{
	return name_;
}

void Feature::setParent(Feature &parent)
{
	parent_ = &parent;
	parent_->addChild(*this);

	// Standard Parent Child Relationship
	Place *addP = &netPattern_->createPlace(
			"p_" + parent_->getAddFeature().getName() + "->"
					+ addFeature_->getName());
	Place *remP = &netPattern_->createPlace(
			"p_" + removeFeature_->getName() + "->"
					+ parent_->getRemoveFeature().getName());

	netPattern_->createArc(*addP, *addFeature_);
	netPattern_->createArc(*removeFeature_, *remP);

	if (type_ == MANDATORY)
	{
		Place *addP2 = &netPattern_->createPlace(
				"p_" + addFeature_->getName() + "->"
						+ parent_->getAddFeature().getName());
		Place *remP2 = &netPattern_->createPlace(
				"p_" + parent_->getRemoveFeature().getName() + "->"
						+ removeFeature_->getName());

		netPattern_->createArc(*addFeature_, *addP2);
		netPattern_->createArc(*remP2, *removeFeature_);
	}
}

Feature &Feature::getParent() const
{
	return *parent_;
}

void Feature::addChild(Feature &child)
{
	children_.push_back(&child);

	// Standard Parent Child Relationship
	Place *addP = &netPattern_->createPlace(
			"p_" + addFeature_->getName() + "->"
					+ children_.back()->getAddFeature().getName());
	Place *remP = &netPattern_->createPlace(
			"p_" + children_.back()->getRemoveFeature().getName() + "->"
					+ removeFeature_->getName());

	netPattern_->createArc(*addFeature_, *addP);
	netPattern_->createArc(*remP, *removeFeature_);

	// If child is mandatory
	if (children_.back()->getType() == MANDATORY)
	{
		Place *addP2 = &netPattern_->createPlace(
				"p_" + children_.back()->getAddFeature().getName() + "->"
						+ addFeature_->getName());
		Place *remP2 = &netPattern_->createPlace(
				"p_" + removeFeature_->getName() + "->"
						+ children_.back()->getRemoveFeature().getName());

		netPattern_->createArc(*addP2, *addFeature_);
		netPattern_->createArc(*removeFeature_, *remP2);
	}
}

const vector<Feature *> &Feature::getChildren() const
{
	return children_;
}

void Feature::setGroup(FeatureGroup &group)
{
	group_ = &group;
	group.addFeature(*this);
}

void Feature::requires(Feature &f)
{
	scream_status(getName() + " requires " + f.getName());
	requires_.push_back(&f);
	f.requiredBy(*this);

	// Add requires relation to net pattern
	scream_status(getName() + ": Try to add place");
	Place *addP = &netPattern_->createPlace(
			"p_" + requires_.back()->getAddFeature().getName() + "->"
					+ addFeature_->getName());
	Place *remP = &netPattern_->createPlace(
			"p_" + removeFeature_->getName() + "->"
					+ requires_.back()->getRemoveFeature().getName());
	scream_status(getName() + ": Adding places successful");

	scream_status(getName() + ": Try to add arcs");
	netPattern_->createArc(*addP, *addFeature_);
	netPattern_->createArc(*removeFeature_, *remP);
	scream_status(getName() + ": Adding arcs successful");
}

const vector<Feature *> &Feature::getRequires() const
{
	return requires_;
}

void Feature::requiredBy(Feature &f)
{
	requiredBy_.push_back(&f);

	// Add requires relation to net pattern
	Place *addP = netPattern_->findPlace(
			"p_" + addFeature_->getName() + "->"
					+ requiredBy_.back()->getAddFeature().getName());
	if (addP)
	{
		scream_error("place " + addP->getName() + " already exists");
		exit(EXIT_FAILURE);
	}
	addP = &netPattern_->createPlace(
			"p_" + addFeature_->getName() + "->"
					+ requiredBy_.back()->getAddFeature().getName());
	Place *remP = netPattern_->findPlace(
			"p_" + requiredBy_.back()->getRemoveFeature().getName() + "->"
					+ removeFeature_->getName());
	if (remP)
	{
		scream_error("place " + addP->getName() + " already exists");
		exit(EXIT_FAILURE);
	}
	remP = &netPattern_->createPlace(
			"p_" + requiredBy_.back()->getRemoveFeature().getName() + "->"
					+ removeFeature_->getName());

	netPattern_->createArc(*addFeature_, *addP);
	netPattern_->createArc(*remP, *removeFeature_);
}

const vector<Feature *> &Feature::getRequiredBy() const
{
	return requiredBy_;
}

#include <iostream>
void Feature::excludes(Feature &f)
{
	for (int i = 0; i < getExcludes().size(); ++i)
		if (getExcludes()[i] == &f)
			return;
	scream_status(getName() + " excludes " + f.getName());
	excludes_.push_back(&f);
	excludes_.back()->excludedBy(*this);

	scream_status(getName() + ": Try adding places");
	// Add exludes relation to net pattern

	Place *exP1 = &netPattern_->createPlace(
			"p_" + removeFeature_->getName() + "->"
					+ excludes_.back()->getAddFeature().getName());
	Place *exP2 = &netPattern_->createPlace(
			"p_" + excludes_.back()->getRemoveFeature().getName() + "->"
					+ addFeature_->getName());
	scream_status(getName() + ": Adding places successful");

	scream_status(getName() + ": Try adding arcs");
	netPattern_->createArc(*removeFeature_, *exP1);
	netPattern_->createArc(*exP2, *addFeature_);
	scream_status(getName() + ": Adding arcs successful");
}

const vector<Feature *> &Feature::getExcludes() const
{
	return excludes_;
}

void Feature::excludedBy(Feature &f)
{
	excludes_.push_back(&f);

	// Add exludes relation to net pattern
	Place *exP1 = &netPattern_->createPlace(
			"p_" + removeFeature_->getName() + "->"
					+ excludes_.back()->getAddFeature().getName());
	Place *exP2 = &netPattern_->createPlace(
			"p_" + excludes_.back()->getRemoveFeature().getName() + "->"
					+ addFeature_->getName());

	netPattern_->createArc(*removeFeature_, *exP1);
	netPattern_->createArc(*exP2, *addFeature_);
}
