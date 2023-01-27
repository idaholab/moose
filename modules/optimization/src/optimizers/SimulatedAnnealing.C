//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimulatedAnnealing.h"

#include "SystemBase.h"

using namespace std;

/*--------------------------------------------------------------------------------------------------
* @brief This subroutine optimizes through simulated annealing
* @param thisSA - the simulated annealing object
*/
void sa_type_base::optimize(sa_type_base &my_sa){
  int i, step;
  double e_curr, t_curr, e_neigh, temp_r;
  e_curr=0.0;
  e_neigh=0.0;
  e_curr=100000.0E0;

  if(my_sa.state_size <= 0){
    mooseError("size/dimensionality of states not specified.");
  }

  //set the cooling function (already set and will do nothing if user custom function)
  set_cooling(my_sa);

  //allocate the neighbor state variables and set the cooling, also set initial energy
  if(sa_comb_type *sa_ptr = dynamic_cast<sa_comb_type*>(&my_sa)){
    // allocate potentially unallocated data
    if(sa_ptr->state_neigh == NULL){
      sa_ptr->state_neigh = new int [my_sa.state_size];
    }
    if(sa_ptr->state_best == NULL){
      sa_ptr->state_best = new int [my_sa.state_size];
    }
    for(i=0; i<my_sa.state_size; i++){
      sa_ptr->state_neigh[i]=sa_ptr->state_curr[i];
      sa_ptr->state_best[i]=sa_ptr->state_curr[i];
    }
    //set energy to current energy
    e_curr=sa_ptr->energy(sa_ptr->state_curr);
    sa_ptr->e_best=e_curr;
  }
  else if(sa_cont_type *sa_ptr = dynamic_cast<sa_cont_type*>(&my_sa)){
    // allocate potentially unallocated data
    if(sa_ptr->state_neigh == NULL){
      sa_ptr->state_neigh = new double [my_sa.state_size];
    }
    if(sa_ptr->state_best == NULL){
      sa_ptr->state_best = new double [my_sa.state_size];
    }
    for(i=0; i<my_sa.state_size; i++){
      sa_ptr->state_neigh[i]=sa_ptr->state_curr[i];
      sa_ptr->state_best[i]=sa_ptr->state_curr[i];
    }
    //set energy to current energy
    e_curr=sa_ptr->energy(sa_ptr->state_curr);
    sa_ptr->e_best=e_curr;
    //set bounds of state if not given
    if(abs(sa_ptr->smin-sa_ptr->smax) < 1.0E-13){
      sa_ptr->smin=1.0E+300;
      sa_ptr->smax=-1.0E+300;
      for(i=0; i<my_sa.state_size; i++){
        if(sa_ptr->smin > sa_ptr->state_curr[i]){
          sa_ptr->smin=sa_ptr->state_curr[i];
        }
        if(sa_ptr->smax < sa_ptr->state_curr[i]){
          sa_ptr->smax=sa_ptr->state_curr[i];
        }
      }
    }
    if(sa_ptr->damping <= 1.0E-15)sa_ptr->damping=abs(sa_ptr->smax-sa_ptr->smin)/2.0E+0;
  }
  else{
    mooseError("bad simulated annealing type.");
  }

  t_curr=my_sa.t_max;
  step=0;
  my_sa.total_steps=0;
  //actual simulated annealing happens in this loop here
  while(step < my_sa.max_step && t_curr > my_sa.t_min){
    my_sa.total_steps++;
    //get a new neighbor and compute energy
    if(sa_comb_type *sa_ptr = dynamic_cast<sa_comb_type*>(&my_sa)){
      sa_ptr->get_neigh(sa_ptr->state_curr, sa_ptr->state_neigh, my_sa.state_size);
      e_neigh=sa_ptr->energy(sa_ptr->state_neigh);
    }
    else if(sa_cont_type *sa_ptr = dynamic_cast<sa_cont_type*>(&my_sa)){
      sa_ptr->get_neigh(sa_ptr->state_curr, sa_ptr->state_neigh, sa_ptr->damping, sa_ptr->smax, sa_ptr->smin, sa_ptr->num_perturb, my_sa.state_size);
      e_neigh=sa_ptr->energy(sa_ptr->state_neigh);
    }
    //check and see if we accept the new energy (lower ergs always accepted)
    temp_r=(double)rand()/(double)RAND_MAX;
    if(temp_r <= accept_prob(e_curr, e_neigh, t_curr)){
      //if we accept then it always counts as a new step
      step++;
      if(sa_comb_type *sa_ptr = dynamic_cast<sa_comb_type*>(&my_sa)){
        for(i=0; i<my_sa.state_size; i++){
          sa_ptr->state_curr[i]=sa_ptr->state_neigh[i];
        }
      }
      else if(sa_cont_type *sa_ptr = dynamic_cast<sa_cont_type*>(&my_sa)){
        for(i=0; i<my_sa.state_size; i++){
          sa_ptr->state_curr[i]=sa_ptr->state_neigh[i];
        }
      }
      e_curr=e_neigh;
    }
    else{
      //otherwise, it has a 50% chance to count as a new step to finish the problem
      //this is especially important for combinatorial problems
      temp_r=(double)rand()/(double)RAND_MAX;
      if(temp_r <= 0.5){
        step++;
      }
    }
    // cool the temperature
    t_curr=my_sa.cool(my_sa, step);
    //if it is the best energy, it's our new best value
    if(e_curr < my_sa.e_best){
      my_sa.e_best=e_curr;
      if(sa_comb_type *sa_ptr = dynamic_cast<sa_comb_type*>(&my_sa)){
        for(i=0; i<my_sa.state_size; i++){
          sa_ptr->state_best[i]=sa_ptr->state_curr[i];
        }
      }
      else if(sa_cont_type *sa_ptr = dynamic_cast<sa_cont_type*>(&my_sa)){
        for(i=0; i<my_sa.state_size; i++){
          sa_ptr->state_best[i]=sa_ptr->state_curr[i];
        }
      }
    }
    //perform non-monotonic adjustment if applicable
    if(!my_sa.mon_cool)t_curr=t_curr*(1.0+(e_curr-my_sa.e_best)/e_curr);
    //rewind to best value of reset is enabled
    if(abs(t_curr) <= my_sa.resvar){
      my_sa.resvar=my_sa.resvar/2.0;
      e_curr=my_sa.e_best;
      if(sa_comb_type *sa_ptr = dynamic_cast<sa_comb_type*>(&my_sa)){
        for(i=0; i<my_sa.state_size; i++){
          sa_ptr->state_curr[i]=sa_ptr->state_best[i];
        }
      }
      else if(sa_cont_type *sa_ptr = dynamic_cast<sa_cont_type*>(&my_sa)){
        for(i=0; i<my_sa.state_size; i++){
          sa_ptr->state_curr[i]=sa_ptr->state_best[i];
          //adjust damping of dynamic damping is enabled
          if(sa_ptr->damp_dyn)sa_ptr->damping=sa_ptr->damping/2.0;
        }
      }
    }
  }

  // select the best state we ended up finding
  if(abs(t_curr) <= my_sa.resvar){
    e_curr=my_sa.e_best;
    if(sa_comb_type *sa_ptr = dynamic_cast<sa_comb_type*>(&my_sa)){
      for(i=0; i<my_sa.state_size; i++){
        sa_ptr->state_curr[i]=sa_ptr->state_best[i];
      }
    }
    else if(sa_cont_type *sa_ptr = dynamic_cast<sa_cont_type*>(&my_sa)){
      for(i=0; i<my_sa.state_size; i++){
        sa_ptr->state_curr[i]=sa_ptr->state_best[i];
      }
    }
  }
}

/*--------------------------------------------------------------------------------------------------
* @brief This subroutine sets the cooling schedule
* @param thisSA - the simulated annealing object
*/
void set_cooling(sa_type_base &my_sa){
  if(my_sa.cool_opt == "LinMult"){
    my_sa.cool=&lin_mult_cool;
  }
  else if(my_sa.cool_opt == "ExpMult"){
    my_sa.cool=&exp_mult_cool;
  }
  else if(my_sa.cool_opt == "LogMult"){
    my_sa.cool=&log_mult_cool;
  }
  else if(my_sa.cool_opt == "QuadMult"){
    my_sa.cool=&quad_mult_cool;
  }
  else if(my_sa.cool_opt == "LinAdd"){
    my_sa.cool=&lin_add_cool;
  }
  else if(my_sa.cool_opt == "QuadAdd"){
    my_sa.cool=&quad_add_cool;
  }
  else if(my_sa.cool_opt == "ExpAdd"){
    my_sa.cool=&exp_add_cool;
  }
  else if(my_sa.cool_opt == "TrigAdd"){
    my_sa.cool=&trig_add_cool;
  }
  else if(my_sa.cool_opt == "custom"){
    //do nothing, it is assumed the user already assgined a custom cooling schedule
    mooseWarning("Using user specified cooling function.");
  }
  else{
    mooseError("bad cooling specification.");
  }
}

/*--------------------------------------------------------------------------------------------------
* @brief This function computes linear multiplicative cooling temperature
* @param thisSA - the simulated annealing object
* @param k - current step
*/
double lin_mult_cool(sa_type_base &my_sa, int k){
  return my_sa.t_max/(1.0E0+my_sa.alpha*k);
}

/*--------------------------------------------------------------------------------------------------
* @brief This function computes natural log exponential multiplicative cooling temperature
* @param thisSA - the simulated annealing object
* @param k - current step
*/
double exp_mult_cool(sa_type_base &my_sa, int k){
  return my_sa.t_max*pow(my_sa.alpha,k);
}

/*--------------------------------------------------------------------------------------------------
* @brief This function computes logarithmic multiplicative cooling temperature
* @param thisSA - the simulated annealing object
* @param k - current step
*/
double log_mult_cool(sa_type_base &my_sa, int k){
  return my_sa.t_max/(1.0E0+my_sa.alpha*log10(k+1.0E0));
}

/*--------------------------------------------------------------------------------------------------
* @brief This function computes quadratic multiplicative cooling temperature
* @param thisSA - the simulated annealing object
* @param k - current step
*/
double quad_mult_cool(sa_type_base &my_sa, int k){
  return my_sa.t_max/(1.0+my_sa.alpha*pow(k*1.0E0,2));
}

/*--------------------------------------------------------------------------------------------------
* @brief This function computes linear additive cooling temperature
* @param thisSA - the simulated annealing object
* @param k - current step
*/
double lin_add_cool(sa_type_base &my_sa, int k){
  return my_sa.t_min+(my_sa.t_max-my_sa.t_min)*(my_sa.max_step*1.0E0-k)/(my_sa.max_step*1.0E0);
}

/*--------------------------------------------------------------------------------------------------
* @brief This function computes quadratic additive cooling temperature
* @param thisSA - the simulated annealing object
* @param k - current step
*/
double quad_add_cool(sa_type_base &my_sa, int k){
  return my_sa.t_min+(my_sa.t_max-my_sa.t_min)*pow((my_sa.max_step*1.0E0-k)/(my_sa.max_step*1.0E0),2);
}

/*--------------------------------------------------------------------------------------------------
* @brief This function computes exponential additive cooling temperature
* @param thisSA - the simulated annealing object
* @param k - current step
*/
double exp_add_cool(sa_type_base &my_sa, int k){
  return my_sa.t_min+(my_sa.t_max-my_sa.t_min)/(1.0E0+exp(2.0E0*log(my_sa.t_max-my_sa.t_min)*(k-0.5E0*my_sa.max_step)/(my_sa.max_step*1.0E0)));
}

/*--------------------------------------------------------------------------------------------------
* @brief This function computes trigonometric additive cooling temperature
* @param thisSA - the simulated annealing object
* @param k - current step
*/
double trig_add_cool(sa_type_base &my_sa, int k){
  return my_sa.t_min+0.5E0*(my_sa.t_max-my_sa.t_min)*(1.0E0+cos(k*M_PI/(my_sa.max_step*1.0E0)));
}

/*--------------------------------------------------------------------------------------------------
* @brief This function gets a new neighbor state for a combinatorial problem
* @param thisSA - the combinatorial simulated annealing object
* @param s_curr - the current state
* @param s_neigh - the neighbor state
* @param size_state - dimensionality of the state
*/
void sa_comb_type::get_neigh(int *s_curr, int *s_neigh, int size_state){
  int i, j1, j2;
  double diff;
  for(i=0; i < size_state; i++){
    s_neigh[i]=s_curr[i];
  }
  diff=0.0;
  while(abs(diff) < 1.0E-15){
    j1=(rand() % size_state);
    j2=(rand() % size_state);
    //set the new neighbor by swapping those points
    s_neigh[j1]=s_curr[j2];
    s_neigh[j2]=s_curr[j1];
    diff=0.0;
    //make sure they're actually different
    for(i=0; i < size_state; i++){
      diff+=abs(1.0*s_neigh[i]-1.0*s_curr[i]);
    }
  }
}

/*--------------------------------------------------------------------------------------------------
* @brief This function gets a new neighbor state for a continuous annealing problem
* @param s_curr - the current state
* @param s_neigh - the neighbor state
* @param damping - the damping factor
* @param smax - maximum state value
* @param smin - minimum state value
* @param size_state - dimensionality of the state
*/
void sa_cont_type::get_neigh(double *s_curr, double *s_neigh, double damping, double smax, double smin, int num_perturb, int size_state){
  int i, temp_i, j;
  double temp_r;
  int * perturb_locs = NULL;
  for(i=0; i < size_state; i++){
    s_neigh[i]=s_curr[i];
  }
  if(num_perturb <= 0 || num_perturb >= size_state){
    //perturb all parameters
    for(i=0; i < size_state; i++){
      temp_r=(double)rand()/(double)RAND_MAX;
      //perturb the state
      s_neigh[i]=s_curr[i]+(1.0-2.0*temp_r)*damping;

      //make sure it doesn't go out of bounds
      if(s_neigh[i] >= smax){
        s_neigh[i]=smax;
      }
      else if(s_neigh[i] <= smin){
        s_neigh[i]=smin;
      }
    }
  }
  else{
    perturb_locs = new int [num_perturb];
    //perturb num_perturb parameters
    for(i=0; i < num_perturb; i++){
      perturb_locs[i]=-1;
    }
    i=0;
    while(i <= num_perturb){
      //get a random index for the parameters
      temp_i=rand() % size_state;
      j=i;
      for(j=0; j < i; j++){
        if(perturb_locs[j] == temp_i)break;
      }
      //if we exited early then we already found the index so we don't use that index
      if(j == i){
        perturb_locs[j]=temp_i;
        i++;
      }
    }
    for(j=0; j < num_perturb; j++){
      i=perturb_locs[j];
      //perturb the state
      temp_r=(double)rand()/(double)RAND_MAX;
      s_neigh[i]=s_curr[i]+(1.0-2.0*temp_r)*damping;

      //make sure it doesn't go out of bounds
      if(s_neigh[i] >= smax){
        s_neigh[i]=smax;
      }
      else if(s_neigh[i] <= smin){
        s_neigh[i]=smin;
      }
    }
  }
}

/*--------------------------------------------------------------------------------------------------
* @brief This function computes the acceptance probability
* @param e_current - the current energy
* @param e_current - the neighboring energy
* @param e_current - the current temperature
*/
double accept_prob(double e_current,double e_neigh,double t_current){
  double delta_e, aprob;

  delta_e=e_neigh-e_current;
  if(-delta_e/t_current <= -700.0E0){
    aprob=0.0;
  }
  else if(-delta_e/t_current >= 700.0E0){
    aprob=10.0;
  }
  else{
    aprob=exp(-delta_e/t_current);
  }
  if(delta_e <= 0.0)aprob=10.0;
  if(isnan(aprob))aprob=0.0;
  return aprob;
}

/*--------------------------------------------------------------------------------------------------
* @brief This function clears/resets an allocated simulated annealing object
* @param thisSA - the simulated annealing object
*/
void sa_type_base::clear(sa_type_base &my_sa){

  // reset to default all variables
  my_sa.state_size=0;
  my_sa.max_step=100;
  my_sa.total_steps=0;
  my_sa.alpha=1.0E-2;
  my_sa.t_max=1.0E+2;
  my_sa.t_min=0.0E0;
  my_sa.e_best=1.0E+307;
  my_sa.cool_opt="LinAdd";
  my_sa.mon_cool=true;
  my_sa.resvar=0.0E0;
  my_sa.cool=NULL;

  // combinatorial or continuous specific defaulting
  if(sa_comb_type *sa_ptr = dynamic_cast<sa_comb_type*>(&my_sa)){
    sa_ptr->state_curr = NULL;
    sa_ptr->state_neigh = NULL;
    sa_ptr->state_best = NULL;
    sa_ptr->energy = NULL;
  }
  else if(sa_cont_type *sa_ptr = dynamic_cast<sa_cont_type*>(&my_sa)){
    sa_ptr->state_curr = NULL;
    sa_ptr->state_neigh = NULL;
    sa_ptr->state_best = NULL;
    sa_ptr->damping=0.0E0;
    sa_ptr->smin=0.0E0;
    sa_ptr->smax=0.0E0;
    sa_ptr->damp_dyn=false;
    sa_ptr->num_perturb=0;
    sa_ptr->energy = NULL;
  }
}