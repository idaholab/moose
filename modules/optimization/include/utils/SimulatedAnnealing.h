//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include <string>
#include <cmath>

//the base simulated annealing solver type
class sa_type_base{
    public:
        // number of values in a state
        int state_size=0;
        // maximum number of SA iterations
        int max_step=100;
        // total number of steps it took
        int total_steps=0;
        // alpha value for cooling
        double alpha=1.0E-2;
        // maximum temperature
        double t_max=1.0E+2;
        // minimum temperature
        double t_min=0.0E0;
        // best energy
        double e_best=1.0E+307;
        // cooling option
        std::string cool_opt="LinAdd";
        // whether cooling is monotonic or not
        bool mon_cool=true;
        // the initial temperature to start resetting the current state to the best found state
        //    when this temperature is reached. This temperature is halved every-time it is reached.
        double resvar=0.0E0;
        // Cooling schedule, to be set by the cooling option
        double (*cool)(sa_type_base &my_sa, int k);
        // optimization subroutine using simulated annealing
        void optimize(sa_type_base &my_sa);
        // virtual energy function to enable polymorphism (and later be overloaded)
        virtual void energy(){};
        // function to clear an allocated object
        void clear(sa_type_base &my_sa);
};

// Combinatorial simulated annealing type
class sa_comb_type: public sa_type_base {
    public:
        // combinatorial problem state array (for perturbing combinatorial problems)
        int * state_curr = NULL;
        // combinatorial problem neighbor array after perturbation
        int * state_neigh = NULL;
        // best energy state
        int * state_best = NULL;
        // energy calculation. to be defined by the program using this utility
        double (*energy)(int *state_val);
        // neighbor retrieval
        void get_neigh(int *s_curr, int *s_neigh, int size_state);
};

// continuous simulated annealing type
class sa_cont_type: public sa_type_base{
    public:
        // continuous problem state array (for perturbing continuous problem values)
        double * state_curr = NULL;
        // continuous problem neighbor array after perturbation
        double * state_neigh = NULL;
        // best energy state
        double * state_best = NULL;
        // damping factor
        double damping=0.0E0;
        // upper and lower bounds, will be set to bounds of initial state if not changed
        double smin=0.0E0, smax=0.0E0;
        // flag for dynamic damping, i.e. if true damping will reduce by a factor of 2 each time the
        //    resvar value is found
        bool damp_dyn=false;
        // Number of parameters to perturb for each neighbor
        int num_perturb=0;
        // energy calculation. to be defined by the program using this utility
        double (*energy)(double *state_val);
        // neighbor retrieval
        void get_neigh(double *s_curr, double *s_neigh, double damping, double smax, double smin, int num_perturb, int size_state);
};

// function to set the cooling schedule
void set_cooling(sa_type_base &my_sa);

// optional cooling schedules
double lin_mult_cool(sa_type_base &my_sa, int k);
double exp_mult_cool(sa_type_base &my_sa, int k);
double log_mult_cool(sa_type_base &my_sa, int k);
double quad_mult_cool(sa_type_base &my_sa, int k);
double lin_add_cool(sa_type_base &my_sa, int k);
double quad_add_cool(sa_type_base &my_sa, int k);
double exp_add_cool(sa_type_base &my_sa, int k);
double trig_add_cool(sa_type_base &my_sa, int k);

//acceptance probability
double accept_prob(double e_current,double e_neigh,double t_current);