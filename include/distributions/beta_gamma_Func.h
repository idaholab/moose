#ifndef BETA_GAMMA_FUNC_H
#define BETA_GAMMA_FUNC_H

#include <iostream>
#include <cmath> // to use erfc error function

/*
 *      this file contains auxiliary functions for the beta and gamma distributions
 *   source: Numerical Recipes in C++ 3rd edition
 */

double gammp(double a, double x);

double gammaFunc(double x);

double betaInc(double a, double b, double x);

double betaFunc(double alpha, double beta);


#endif /* BETA_GAMMA_FUNC_H */
