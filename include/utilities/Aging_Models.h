/*
 *  Aging_Models.h
 *
 *  This function perform the integration
 *
 *  Created on: June 19, 2014
 */

#ifndef AGING_MODELS_H_
#define AGING_MODELS

void agingModels(std::vector<double> input_1, std::vector<double> input_2, std::vector<std::vector<double> > output);

// Input_1 and input_2 are the temporal profiles of temperature and pressure for the particular passive component
// Note that more input might be needed,

// output is the temporal profile of the seven Markov state probabilities


#endif /* AGING_MODELS */

