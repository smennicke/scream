/*
 * Stage.cpp
 *
 *  Created on: Sep 23, 2013
 *      Author: stephan
 */

#include <iostream>
#include <sstream>

#include "Feature.h"
#include "Stage.h"
#include "tools.h"

using namespace std;

Stage::Stage() :
	net_(NULL)
{
}

Stage::~Stage() {
}

void Stage::push_back(Feature *f) {
	features_.push_back(f);
}

Feature *Stage::pop_back() {
	if (features_.size() == 0)
		exit(EXIT_FAILURE);
	Feature *f = features_[features_.size() - 1];
	features_.pop_back();
//	std::cout << "removed last feature of " << this << endl;

	return f;
}

void Stage::push_front(Feature *f) {
	int size = features_.size();
//	std::cout << "before (" << this << "): " << size << endl;
	features_.insert(features_.begin(), f);
//	std::cout << "after: " << features_.size() << endl;
}

Feature *Stage::pop_front() {
	if (features_.size() == 0)
		exit(EXIT_FAILURE);
	Feature *f = features_[0];
	features_.erase(features_.begin());

	return f;
}

PetriNet &Stage::getWorkflow(int stage, set<Feature *> added) {
	if (net_ != NULL) {
		delete net_;
//		net_ = NULL;
	}
	net_ = new PetriNet();
	PetriNet &net = *net_;

	// Prepare workflow structure
	stringstream in, out;
	in << "i_" << stage+1;
	out << "o_" << stage+1;
	stringstream begins, ends;
	begins << "begin_" << stage+1;
	ends << "end_" << stage+1;

	Place &i = net.createPlace(in.str());
	if (stage == 0)
		i.setTokenCount(1);
	Place &o = net.createPlace(out.str());
	i_ = i.getName();
	o_ = o.getName();
	Transition *begin = &net.createTransition(begins.str());
	Transition *end = &net.createTransition(ends.str());

	net.createArc(i, *begin);
	net.createArc(*end, o);

	// Add features to stage
	for (int i = 0; i < features_.size(); ++i) {
		synchronousProduct(net, features_[i]->getPattern());
		Place *p;
//		stringstream s;
//		s << features_[i] << ": " << features_[i]->getName() <<
//				" (" << ( features_[i]->isRoot() ? "is root)" : "is not root)" );
//		scream_error(s.str());
		if (!features_[i]->isRoot() &&
				(p = net.findPlace("p_" + features_[i]->getRemoveFeature().getName())) != NULL
				&& !added.count(&features_[i]->getParent()))
			net.deletePlace(*p);


		Place *in = net.findPlace(features_[i]->getInputFeature().getName());
		Place *out = net.findPlace(features_[i]->getOutputFeature().getName());

		net.createArc(*begin, *in);
		net.createArc(*out, *end);
	}

	return net;
}

const string Stage::toString() const {
	string result = "{";
	for (int i = 0; i < features_.size(); ++i)
		result += " " + features_[i]->getName();
	return result + " }";
}

