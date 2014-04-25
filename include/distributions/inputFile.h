/*
 * InputFile.h
 *
 *  Created on: Oct 6, 2011
 *      Author: MANDD
 */

#ifndef INPUTFILE_H_
#define INPUTFILE_H_

#include <sstream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <string>


void LoadData(double** data, int dimensionality, int cardinality, std::string filename);


#endif /* INPUTFILE_H_ */
