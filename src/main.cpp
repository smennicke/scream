/*
 ============================================================================
 Name        : main.cpp
 Author      : Stephan Mennicke
 Version     : 1.0
 Copyright   : Copyright 2013 Stephan Mennicke (LGPL)
 Description : SCREAM is a Staged Configuration tool for fEAture Models
 ============================================================================
 */

#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "pnapi/pnapi.h"

using namespace std;
using namespace pnapi;

#include "cmdline.h"
#include "config.h"
#include "Feature.h"
#include "FeatureModel.h"
#include "StageModel.h"

#include "splot-parser.h"
#include "types.h"
#include "tools.h"

/*
 * Global Variables
 */
gengetopt_args_info args_info;
ostream *log, *res;
istream *inp;
string filename, basefilename;
stringstream productSeq;
int fcount;
string basepath;
time_t start_scream, end_scream;
time_t start_alg, end_alg;
time_t start_snd, end_snd;

// Parser & Lexer
extern int splot_yyparse();
extern FILE *splot_yyin;
FeatureModel *fm;

int main(int argc, char **argv)
{
	time(&start_scream);
	/*
	 * 0. Evaluate Parameters
	 */
	fcount = 0;
	Strategy oStrat;
	InnerStrategy iStrat;

	if (cmdline_parser(argc, argv, &args_info) != 0)
	{
		scream_error("Could not parse command parameters");
		return EXIT_FAILURE;
	}

	if (args_info.inputs_num == 0)
	{
		// At least one input file must be given
		scream_error("No file given");
		return EXIT_FAILURE;
	}
	else
	{
		filename = string(args_info.inputs[0]);
		basefilename = filename.substr(0, (filename.find_last_of('.')));

		if (args_info.basepath_given)
			basepath = string(args_info.basepath_arg);
		else
			basepath = "";

		// Logging
		log = NULL;

		if (args_info.Log_given)
		{
			// Print log information to stderr
			log = &cerr;
		}
		else if (args_info.log_given)
		{
			ofstream &logfile = *(new ofstream());
			if (args_info.log_arg == NULL)
			{
				stringstream file;
				file << basefilename + ".log";
				logfile.open(file.str().c_str());
			}
			else
			{
				logfile.open(args_info.log_arg);
			}

			log = &logfile;
		}

		/*
		 * Interactive Mode overwrites all previous log options
		 */
		if (args_info.interactive_given)
		{
			static_cast<ofstream *>(log)->close();
			log = &cerr;
		}

		// Result
		res = NULL;
		if (args_info.Result_given)
			res = &cout;
		if (args_info.result_given)
		{
			ofstream &resfile = *(new ofstream());
			if (args_info.result_arg == NULL)
			{
				stringstream file;
				file << basefilename << ".scream";
				resfile.open(file.str().c_str());
			}
			else
				resfile.open(args_info.result_arg);

			res = &resfile;
		}

		// Outer Strategy Parameter
		switch (args_info.outer_strategy_arg)
		{
		case outer_strategy_arg_forwards:
			scream_status("forwards strategy chosen");
			oStrat = FORWARDS;
			break;
		case outer_strategy_arg_backwards:
		default:
			scream_status("backwards strategy chosen");
//			scream_error("backwards strategy is error prone, SCREAM may abort with an error message (please ignore), choose the forwards strategy");
			oStrat = BACKWARDS;
			break;
		}

		// Inner Strategy Parameter
		switch (args_info.inner_strategy_arg)
		{
		case inner_strategy_arg_first:
			scream_status("first strategy chosen");
			iStrat = FIRST;
			break;
		case inner_strategy_arg_last:
		default:
			scream_status("last strategy chosen");
			iStrat = LAST;
			break;
		}

		// Product input
		if (args_info.product_given)
		{
			ifstream inputfile;
			if (args_info.product_arg == NULL)
			{
				scream_error("No product file given");
				return EXIT_FAILURE;
			}
			else
			{
				scream_status(
						"input filename given: '"
								+ string(args_info.product_arg) + "'");
				inputfile.open(args_info.product_arg);
			}
			if (inputfile.is_open())
			{
				string line;
				while (getline(inputfile, line))
					productSeq << line;
			}
			else
			{
				scream_error(
						"Were not able to read from product sequence file");
				return EXIT_FAILURE;
			}

			scream_status("Got product sequence (" + productSeq.str() + ")");
			inputfile.close();
		}
		else
		{
			scream_status("Using standard feature sequence");
		}
	}

	/*
	 * 1.1 Parse and Build Feature Model (FM)
	 */
	if ((splot_yyin = fopen(filename.c_str(), "r")) == NULL)
	{
		scream_error("Could not open file '" + filename + "'");
		return EXIT_FAILURE;
	}
	if (splot_yyparse())
	{
		scream_error("Parse Error");
		return EXIT_FAILURE;
	}

	stringstream note;
	note << "Parsed and built Feature Model with " << fcount << " ("
			<< fm->size() << ") features";
	scream_status(note.str());

	/*
	 * 1.2 Preliminary Analysis on FM
	 */
	if (fm->getRootFeature().getType() != MANDATORY)
	{
		scream_error("Root feature is not mandatory");
		return EXIT_FAILURE;
	}
	scream_status("Root feature is mandatory");

	/*
	 * 2. Enter a Feature Sequence
	 */
	if (fm->checkConfiguration(productSeq.str()))
	{
		scream_status("Feature sequence is consistent with FM");
	}
	else
	{
		scream_error("Feature sequence is not consistent with FM");
		return EXIT_FAILURE;
	}

	/*
	 * 3. Create Stage Model (SM)
	 */
	vector<Feature *> config;
	istringstream ss(productSeq.str());
	while (ss)
	{
		string sub;
		Feature *f;
		ss >> sub;
		if (sub.empty())
			continue;
		if ((f = fm->findFeature(sub)) != NULL)
		{
			config.push_back(f);
		}
	}

	time(&start_alg);
	StageModel &sm = fm->computeStageModel(config, oStrat, iStrat);
	time(&end_alg);

	/*
	 * 4. SM Correctness (Built in SM Creation)
	 */
	if (sm.isSound())
	{
		cout << " +++++ Found sound stage model +++++ " << endl << " +++++ "
				<< sm.toString() << " +++++ " << endl;
	}
	else
	{
		cout << " ----- Did not find a sound stage model ----- " << endl;
	}

	time(&end_scream);
	/*
	 * 5. Output
	 *   - Sound SM
	 *   - Lazy sound & no dead transitions WFM (Witness)
	 *   - FM Class = SM Class
	 */
	scream_output(sm.getPetriNet());
	if (res != NULL)
	{
		scream_resultfile(&sm);
	}
	if (args_info.csv_given)
	{
		scream_csv(&sm);
	}

	/*
	 * 6. Clean-up
	 */
	if (fm != NULL)
		delete fm;
	if (&sm != NULL)
		delete &sm;

	if (args_info.log_given)
	{
		static_cast<ofstream *>(log)->close();
	}
	if (args_info.result_given)
	{
		static_cast<ofstream *>(res)->close();
	}
	// Freeing the Commandline Parser
	cmdline_parser_free(&args_info);

	return EXIT_SUCCESS;
}
