/*
 * tools.h
 *
 *  Created on: Sep 23, 2013
 *      Author: stephan
 */

#ifndef TOOLS_H_
#define TOOLS_H_

#include <iostream>
#include <string>
#include <vector>
#include "pnapi/pnapi.h"

using std::ostream;
using std::string;
using std::vector;
using namespace pnapi;

class FeatureModel;
class StageModel;


// Synchronous Product, which merges same named transitions and places
void synchronousProduct(PetriNet &net1, const PetriNet &net2);

// Workflow Module Composition, which merges two nets and cleans up false places
void wfmComposition(PetriNet &net1, PetriNet &net2, vector<string> &toBeDeleted, int stage);

// Lazy Soundness Check
bool isLazySound(const PetriNet &net, const string &oName);

// No Dead Transitions Check
bool hasNoDeadTransitions(const PetriNet &net);

/*
 * Verbose Output Functions
 */

void scream_status(const string &msg);

void scream_error(const string &msg);

void scream_output(const PetriNet &pn);

void force_output(const PetriNet &pn, const string &filename);

void scream_resultfile(StageModel *sm);

void scream_csv(StageModel *sm);


#endif /* TOOLS_H_ */
