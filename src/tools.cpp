/*
 * tools.cpp
 *
 *  Created on: Sep 23, 2013
 *      Author: stephan
 */

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>

#include "FeatureModel.h"
#include "StageModel.h"

#include "cmdline.h"
#include "config.h"
#include "tools.h"
#include "verbose.h"

using namespace std;

/*
 * External Global Variables
 */
extern ostream *log, *res;
extern gengetopt_args_info args_info;
extern string basefilename;
extern string filename;
extern stringstream productSeq;
extern FeatureModel *fm;
extern string basepath;
extern time_t start_scream, end_scream;
extern time_t start_alg, end_alg;
extern time_t start_snd, end_snd;

void synchronousProduct(PetriNet &net1, const PetriNet &net2)
{
	PNAPI_FOREACH(t, net2.getTransitions())
	{
		Transition *t1 = net1.findTransition((*t)->getName());
		if (t1 == NULL)
		{
			t1 = &net1.createTransition((*t)->getName());
		}

		// Iterate over preset of t
		PNAPI_FOREACH (p, (*t)->getPreset())
		{
			Place *p1 = net1.findPlace((*p)->getName());
			if (p1 == NULL)
				p1 = &net1.createPlace((*p)->getName(),
						(static_cast<Place *>(*p))->getTokenCount());
			net1.createArc(*p1, *t1);
		}
		// Iterate over postset of t
		PNAPI_FOREACH(p, (*t)->getPostset())
		{
			Place *p1 = net1.findPlace((*p)->getName());
			if (p1 == NULL)
				p1 = &net1.createPlace((*p)->getName());
			net1.createArc(*t1, *p1);
		}
	}
}

void wfmComposition(PetriNet &net1, PetriNet &net2, vector<string> &toBeDeleted,
		int stage)
{
	/*
	 * 1. Delete unnecessary places in net2
	 */
	Place *p;
	for (int i = 0; i < toBeDeleted.size(); ++i)
	{
		if ((p = net2.findPlace(toBeDeleted[i])) != NULL)
			net2.deletePlace(*p);
	}

	set<Place *> places = net2.getPlaces();
	PNAPI_FOREACH(pp, places)
	{
		if ((*pp)->getPostset().size()
				== 0&& (p = net1.findPlace((*pp)->getName())) != NULL){
		toBeDeleted.push_back((*pp)->getName());
		net1.deletePlace(*p);
		net2.deletePlace(**pp);
	}
}

		/*
		 * 4. Identify Workflow Places and merge both nets
		 */
	synchronousProduct(net1, net2);
	stringstream in, out;
	in << "i_" << stage + 1;
	out << "o_" << stage;
	Place *inew = net1.findPlace(in.str());
	Place *o = net1.findPlace(out.str());

	if (o != NULL)
	{
		PNAPI_FOREACH(t, inew->getPostset())
		{
			net1.createArc(*o, **t);
		}
		net1.deletePlace(*inew);
	}

	places = net1.getPlaces();
	PNAPI_FOREACH(p, places)
	{
		if ((*p)->getPreset().size() == 0 && (*p)->getTokenCount() < 1)
		{
			toBeDeleted.push_back((*p)->getName());
			net1.deletePlace(**p);
		}
	}
}

bool isLazySound(const PetriNet &net, const string &oName)
{
//	status("Preliminary Checks (oName='" + oName + "')");

	/*
	 * 0. Check for output place
	 */
	scream_status(" ===== Lazy Soundness Check (weak termination) ===== ");

	/*
	 * 1. Write net to LoLA file
	 */
	stringstream fname;
	fname << basefilename << ".lola";
	std::ofstream file;
	file.open(fname.str().c_str());
	file << io::lola << net;
	file.close();

	/*
	 * 2. Write Task file
	 */
	stringstream fname2;
	fname2 << basefilename << ".task";
	file.open(fname2.str().c_str());
	file << "FORMULA ALLPATH EVENTUALLY ( " << oName << " = 1 )";
	file.close();

	/*
	 * 3. Let LoLA run
	 */
	string cmd = basepath + "lola-modelchecking " + fname.str()
			+ " -a -S -P 2> /dev/null";
	time(&start_snd);
	FILE *pipe = popen(cmd.c_str(), "r");
	if (pipe == NULL)
	{
		scream_error("Could not run LoLA to compute weak termination");
		return false;
	}
	char temp[1000];
	stringstream result;
	while (fgets(temp, 1000, pipe))
	{
		result << temp;
	}
	pclose(pipe);
	time(&end_snd);

//	status("LoLA Result");
//	status(result.str());
//	status("End of LoLA Results");

	/*
	 * 4. Clean-up (delete created files)
	 */

	/*
	 * 5. Parse LoLA result
	 */
	string b =
			result.str().find("result: true") != result.str().npos ?
					"sound" : "unsound";
	scream_status(" ===== END Lazy Soundness Check (" + b + ") ===== ");

	if (b == "sound")
		return true;

	if (args_info.interactive_given)
	{
		scream_status("Interactive Mode: Press Enter to Continue");
		cin.ignore();
	}

	return false;
}

bool hasNoDeadTransitions(const PetriNet &net)
{
	scream_status(" ===== BEGIN Dead Transitions Check ===== ");

	/*
	 * 1. Write to LoLA file
	 */
	stringstream fname;
	fname << basefilename << ".lola";
	std::ofstream file;
	file.open(fname.str().c_str());
	file << io::lola << net;
	file.close();

	/*
	 * 2. Check for Dead Transitions
	 */PNAPI_FOREACH(t, net.getTransitions())
	{
		/*
		 * 2.1 Write Task file for t
		 */
		stringstream tname;
		tname << basefilename << "." << (*t)->getName() << ".task";
		file.open(tname.str().c_str());
		file << "ANALYSE TRANSITION " << (*t)->getName();
		file.close();

		/*
		 * 2.2 Call LoLA
		 */
		stringstream rfile;
		rfile << basefilename << ".lola-result";
		string cmd = basepath + "lola-deadtransition " + fname.str() + " -a"
				+ tname.str() + " -S -P -r" + rfile.str() + " 2> /dev/null";
//		scream_status("Try command: " + cmd);
		FILE *pipe = popen(cmd.c_str(), "r");
		if (pipe == NULL)
		{
			scream_error(
					"Could not run LoLA to compute dead transition for transition "
							+ (*t)->getName());
//			return false;
			scream_error(cmd.c_str());
			exit(EXIT_FAILURE);
		}
		char temp[1000];
		stringstream result;
		while (fgets(temp, 1000, pipe))
		{
			result << temp;
		}
		pclose(pipe);
		FILE *resultf = fopen(rfile.str().c_str(), "r");
		if (resultf == NULL)
		{
			scream_error(
					"Could not open LoLA result file of dead transitions check for transition "
							+ (*t)->getName());
//			return false;
			exit(EXIT_FAILURE);
		}
		stringstream result0;
		while (fgets(temp, 1000, resultf))
			result0 << temp;
		fclose(resultf);
//		status("LoLA Result");
//		status(result.str());
//		status("End of LoLA Results");

		/*
		 * 2.3 Parse LoLA Output
		 */
		if (result0.str().find("result = true") == result.str().npos)
		{
			scream_status("Transition " + (*t)->getName() + " is dead");
//			scream_status("LoLA Output:");
//			scream_status(result.str());
			scream_status(" ===== END Dead Transitions Check ===== ");
//			force_output(net, basefilename + "_" + (*t)->getName() + "_isDead");

			if (args_info.interactive_given)
			{
				scream_status("Interactive Mode: Press Enter to Continue");
				cin.ignore();
			}

			return false;
		}
		else
		{
			scream_status("Transition " + (*t)->getName() + " is live");
		}

		cmd = "rm -rf " + tname.str() + " " + rfile.str() + " *_isDead*";
		system(cmd.c_str());
	}

	scream_status(" ===== END Dead Transitions Check ===== ");

	// 3. No Dead Transitions
	return true;
}

/*
 * Verbose Output Functions
 */

void scream_status(const string &msg)
{
//	if (log == NULL)
	return;
	(*log) << PACKAGE_NAME << " Status: " << msg << endl;
}

void scream_error(const string &msg)
{
	cerr << PACKAGE_NAME << " Error: " << msg << endl;
}

void scream_output(const PetriNet &pn)
{
	ofstream file;
	stringstream cmd;
	for (int i = 0; i < args_info.output_given; ++i)
	{
		switch (args_info.output_arg[i])
		{
		case output_arg_lola:
			file.open(string(basefilename + ".lola").c_str());
			file << io::lola << pn;
			file.close();
			break;
		case output_arg_dot:
			file.open(string(basefilename + ".dot").c_str());
			file << io::dot << pn;
			file.close();
			break;
		case output_arg_png:
			file.open(string(basefilename + ".dot").c_str());
			file << io::dot << pn;
			file.close();

			cmd.clear();
			cmd << "dot -Tpng " << basefilename << ".dot > " << basefilename
					<< ".png";
			if (system(cmd.str().c_str()))
				scream_status("(PNG Output) Call graphviz-dot successful");

			break;
		case output_arg_pdf:
			file.open(string(basefilename + ".dot").c_str());
			file << io::dot << pn;
			file.close();

			cmd.clear();
			cmd << "dot -Tpdf " << basefilename << ".dot > " << basefilename
					<< ".pdf";
			if (system(cmd.str().c_str()))
				scream_status("(PDF Output) Call graphviz-dot successful");

			break;
		default:
			break;
		}
	}
}

void force_output(const PetriNet &pn, const string &filename)
{
	string fname = (filename.empty() ? basefilename : filename);

	ofstream file;
	file.open(string(fname + ".lola").c_str());
	file << io::lola << pn;
	file.close();
}

void scream_resultfile(StageModel *sm)
{
	ostream &out = *res;
	out << endl;
	out << " +------ Scream Result File ------ " << endl;
	out << " |               FM: " << filename << endl;
	out << " |        #Features: " << fm->size() << endl;
	out << " |             CTCR: " << fm->ctcr() << " %" << endl;
	out << " |         Net size: "
			<< fm->getPetriNet().getArcs().size()
					+ fm->getPetriNet().getPlaces().size()
					+ fm->getPetriNet().getTransitions().size() << endl;
	out << " +-------------------------------- " << endl;
	out << " |   Given sequence: " << productSeq.str() << endl;
	out << " | #Tried sequences: " << sm->tried() << endl;
	out << " |    Last sequence: " << sm->getProductSequence() << endl;
	out << " |   Outer Strategy: "
			<< (args_info.outer_strategy_arg == outer_strategy_arg_forwards ?
					"forwards" : "backwards") << endl;
	out << " |   Inner Strategy: "
			<< (args_info.inner_strategy_arg == inner_strategy_arg_last ?
					"last" : "first") << endl;
	out << " |     #SMs visited: " << sm->visited() << endl;
	out << " +-------------------------------- " << endl;
	out << " |         sound SM: " << (sm->isSound() ? "yes" : "no") << endl;
	out << " |  #Features added: " << sm->numFeatures() << endl;
	out << " |          #Stages: " << sm->size() << endl;
	out << " |   Features/Stage: " << sm->ratio() << endl;
	out << " |        Max Stage: " << sm->max() << endl;
	out << " |        Min Stage: " << sm->min() << endl;
	out << " +-------------------------------- " << endl;
	int pnsize = sm->getPetriNet().getPlaces().size()
			+ sm->getPetriNet().getTransitions().size()
			+ sm->getPetriNet().getArcs().size();
	out << " |             |P|: " << sm->getPetriNet().getPlaces().size()
			<< endl;
	out << " |             |T|: " << sm->getPetriNet().getTransitions().size()
			<< endl;
	out << " |             |F|: " << sm->getPetriNet().getArcs().size() << endl;
	out << " |            |PN|: " << pnsize << endl;
	out << " +-------------------------------- " << endl;
	out << " | " << sm->toString() << endl;
	out << " +-------------------------------- " << endl;
	out << " |  Runtime SCREAM: " << difftime(end_scream, start_scream) << endl;
	out << " |    Runtime Alg.: " << difftime(end_alg, start_alg) << endl;
	out << " | Runtime Soundn.: " << difftime(end_snd, start_snd) << endl;
	out << " +-------------------------------- " << endl << endl;
}

/*
 * CSV File Format
 *
 * filename,#features,ctcr,pn-size,strategy,#visited SMs,sound?,#features,#stages,features/stage,min,max,|P|,|T|,|F|,|PN|,scream,algo,lola
 */
#define COMMA ","
void scream_csv(StageModel *sm)
{
	ofstream out;
	if (args_info.csv_arg == NULL)
		out.open((basefilename + ".csv").c_str(), ios::app);
	else
		out.open(args_info.csv_arg, ios::app);

	out << filename << COMMA;
	out << fm->size() << COMMA;
	out << fm->ctcr() << COMMA;
	out << fm->getPetriNet().getArcs().size()
						+ fm->getPetriNet().getPlaces().size()
						+ fm->getPetriNet().getTransitions().size() << COMMA;

	out	<< (args_info.outer_strategy_arg == outer_strategy_arg_forwards ?
					"forwards" : "backwards") << COMMA;
	out << sm->visited() << COMMA;
	out << (sm->isSound() ? "yes" : "no") << COMMA;
	out << sm->numFeatures() << COMMA;
	out << sm->size() << COMMA;
	out << sm->ratio() << COMMA;
	out << sm->max() << COMMA;
	out << sm->min() << COMMA;

	int pnsize = sm->getPetriNet().getPlaces().size()
			+ sm->getPetriNet().getTransitions().size()
			+ sm->getPetriNet().getArcs().size();
	out << sm->getPetriNet().getPlaces().size() << COMMA;
	out << sm->getPetriNet().getTransitions().size() << COMMA;
	out << sm->getPetriNet().getArcs().size() << COMMA;
	out << pnsize << COMMA;

	out << difftime(end_scream, start_scream) << COMMA;
	out << difftime(end_alg, start_alg) << COMMA;
	out << difftime(end_snd, start_snd) << endl;

	out.close();
}

