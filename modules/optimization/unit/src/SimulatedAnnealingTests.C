//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

// header for the global variables and the like
#include "SimulatedAnnealingTests.h"

// traveling salesman test
TEST(SimulatedAnnealingTests, TravelingSalesman)
{
  int i=0;
  int j=0;
  double sort_best=0.0;
  double cust_min=10000.0;
  double cust_max=-10000.0;

  //set random customer locations
  cust_locs = new double*[num_customers];
  for(i=0; i<num_customers; i++){
    cust_locs[i] = new double[prob_dim];
    for(j=0; j<prob_dim; j++){
      cust_locs[i][j]=(double)rand()/(double)RAND_MAX;
      if(cust_min >= cust_locs[i][j])cust_min=cust_locs[i][j];
      if(cust_max <= cust_locs[i][j])cust_max=cust_locs[i][j];
    }
  }

  //for a 1D problem, this is the optimal path length
  if(prob_dim == 1)sort_best=cust_max-cust_min;

  //simulated annealing settings
  ts_simanneal.max_step=10000*num_customers;
  ts_simanneal.t_max=100;
  ts_simanneal.t_min=0;
  ts_simanneal.cool_opt="QuadAdd";
  ts_simanneal.mon_cool=false;
  ts_simanneal.state_curr=new int[num_customers];
  //default initial guess to original order
  ts_simanneal.state_size=num_customers;
  for(i=0; i<num_customers; i++){
    ts_simanneal.state_curr[i]=i;
  }
  //point to a path length function that works with the SA type
  ts_simanneal.energy=&path_len;

  ts_simanneal.optimize(ts_simanneal);

  // check that the simulated annealing result is optimal
  EXPECT_EQ(ts_simanneal.e_best, sort_best);
}

//path length of a given customer ordering
double path_len(int * state_ord){
  int i;
  double comp_val;

  comp_val=0.0;
  for(i=0; i<num_customers-1; i++){
    comp_val+=dist(cust_locs[state_ord[i]], cust_locs[state_ord[i+1]]);
  }
  return comp_val;
}

//distance between two customers
double dist(double *loc1, double *loc2){
  int i;
  double dist_val;

  dist_val=0.0;
  for(i=0; i<prob_dim; i++){
    dist_val+=pow(loc1[i]-loc2[i],2);
  }
  dist_val=sqrt(dist_val);

  return dist_val;
}

TEST(SimulatedAnnealingTests, ContinuousMinimization)
{
  EXPECT_LE(1, 2);
  // ...
}