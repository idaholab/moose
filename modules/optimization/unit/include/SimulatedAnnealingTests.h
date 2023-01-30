//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include <iostream>
#include <string>
#include <math.h>

//simulated annealing capabilities
#include "SimulatedAnnealing.h"

using namespace std;

// traveling salesman simulated annealing object
sa_comb_type ts_simanneal;

// number of customers
int num_customers=20;

//problem dimension
int prob_dim=1;

// customer locations
double ** cust_locs;

//path length of a given customer ordering
double path_len(int * a);

//distance between two customers
double dist(double *loc1, double *loc2);