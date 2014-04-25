/*
 * beta_gamma_Func.h
 *
 *  Created on: Mar 27, 2012
 *      Author: MANDD
 *      this file contains auxiliary functions for the beta and gamma distributions
 *
 *      Tests		: None
 *
 *      Problems	: None
 *      Issues		: None
 *      Complaints	: None
 *      Compliments	: None
 *
 *   source: Numerical Recipes in C++ 3rd edition
 */

#include <iostream>
#include <stdio.h>
#include <math.h>
#include <cmath>	// to use erfc error function


#ifndef BETA_GAMMA_FUNC_H_
#define BETA_GAMMA_FUNC_H_

double gammp(double a, double x);

double gammaFunc(double x);

double betaInc(double a, double b, double x);

double betaFunc(double alpha, double beta);


#endif /* BETA_GAMMA_FUNC_H_ */
