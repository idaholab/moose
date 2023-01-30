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
  // traveling salesman simulated annealing object
  sa_comb_type ts_simanneal;

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
  EXPECT_DOUBLE_EQ(ts_simanneal.e_best, sort_best);
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
  // reference comparisons
  double ref_val;
  double *minlocs;

  // Simulated Annealing object for the functions
  sa_cont_type func_sa;

  minlocs=new double [2];

  // Reference minimum point values
  ref_val=1.5449192781044832E+01+1.4184540054199392E+01;

  // Reference minimum point locations
  minlocs[0]=-1.1260671421437776E+00;
  minlocs[1]= 2.8556531452530787E+00;

  func_sa.max_step=10000;
  func_sa.t_max=100;
  func_sa.t_min=0;
  func_sa.cool_opt="QuadAdd";
  func_sa.mon_cool=false;
  func_sa.smin=-10;
  func_sa.smax=10;
  func_sa.damping=0;
  func_sa.resvar=1;
  func_sa.damp_dyn=true;
  // give random initial guess
  func_sa.state_size=2;
  func_sa.state_curr=new double [func_sa.state_size];
  func_sa.state_curr[0]=(double)rand()/(double)RAND_MAX;
  func_sa.state_curr[0]=func_sa.state_curr[0]*2.0E+1-1.0E+1;
  func_sa.state_curr[1]=(double)rand()/(double)RAND_MAX;
  func_sa.state_curr[1]=func_sa.state_curr[0]*2.0E+1-1.0E+1;

  // give energy function
  func_sa.energy=&comb_func;

  //optimize
  func_sa.optimize(func_sa);
  //check results
  EXPECT_LE(abs(func_sa.e_best-ref_val)/ref_val,1.0E-4);
  EXPECT_LE(abs((func_sa.state_best[0]-minlocs[0])/minlocs[0]),1.0E-3);
  EXPECT_LE(abs((func_sa.state_best[1]-minlocs[1])/minlocs[1]),1.0E-3);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// function 1
double comb_func(double *x){
  return 10.0*sin(x[0])-0.05*(x[0]+2.0)+pow(x[0]-1.0,2)+20.0+1.78E-6*pow(x[1],8)+1.86E-5*pow(x[1],7)
          -3.75E-4*pow(x[1],6)-3.61E-3*pow(x[1],5)+2.55E-2*pow(x[1],4)
          +2.06E-1*pow(x[1],3)-4.85E-1*pow(x[1],2)-3.11E0*x[1]+1.38E0+20.E0;
}