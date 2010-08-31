#include "TransientExecutioner.h"

//Moose includes
#include "Kernel.h"
#include "MaterialFactory.h"
#include "MooseSystem.h"
#include "ComputeJacobian.h"
#include "ComputeResidual.h"

//libMesh includes
#include "implicit_system.h"
#include "nonlinear_implicit_system.h"
#include "transient_system.h"
#include "numeric_vector.h"

// C++ Includes
#include <iomanip>

template<>
InputParameters validParams<TransientExecutioner>()
{
  InputParameters params = validParams<Executioner>();

  params.addParam<Real>("start_time",      0.0,    "The start time of the simulation");
  params.addParam<Real>("end_time",        1.0e30, "The end time of the simulation");
  params.addRequiredParam<Real>("dt", "The timestep size between solves");
  params.addParam<Real>("dtmin",           0.0,    "The minimum timestep size in an adaptive run");
  params.addParam<Real>("dtmax",           1.0e30, "The maximum timestep size in an adaptive run");
  params.addParam<int> ("num_steps",       -1,     "The number of timesteps in a transient run");
  params.addParam<int> ("n_startup_steps", 0,      "The number of timesteps during startup");
  params.addParam<bool>("trans_ss_check",  false,  "Whether or not to check for steady state conditions");
  params.addParam<Real>("ss_check_tol",    1.0e-08,"Whenever the relative residual changes by less than this the solution will be considered to be at steady state.");
  params.addParam<Real>("ss_tmin",         0.0,    "Minimum number of timesteps to take before checking for steady state conditions.");

  return params;
}

TransientExecutioner::TransientExecutioner(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Executioner(name, moose_system, parameters),
   _t_step(moose_system.parameters().set<int> ("t_step") = 0),
   _time(moose_system.parameters().set<Real>("time") = parameters.get<Real>("start_time")),
   _input_dt(parameters.get<Real>("dt")),
   _dt(moose_system.parameters().set<Real>("dt") = 0),
   _end_time(parameters.get<Real>("end_time")),
   _dtmin(parameters.get<Real>("dtmin")),
   _dtmax(parameters.get<Real>("dtmax")),
   _num_steps(parameters.get<int>("num_steps")),
   _n_startup_steps(parameters.get<int>("n_startup_steps")),
   _trans_ss_check(parameters.get<bool>("trans_ss_check")),
   _ss_check_tol(parameters.get<Real>("ss_check_tol")),
   _ss_tmin(parameters.get<Real>("ss_tmin")),
   _converged(true)
{
  _moose_system.reinitDT();
}

void
TransientExecutioner::execute()
{
  // Start time loop...
  while(keepGoing()) 
  {
    takeStep();
  }
}

void
TransientExecutioner::takeStep()
{
  // If this is the first step
  if(_t_step == 0)
  {
    _t_step = 1;
    _dt = _input_dt;
  }

  if(_converged)
  {
    // Update backward time solution vectors
    _moose_system.copy_old_solutions();

    // Update backward material data structures
    _moose_system.updateMaterials();
  }    

  _moose_system.getNonlinearSystem()->update();

  Real dt_cur = computeDT();

  // Don't let the time step size exceed maximum time step size
  if(dt_cur > _dtmax)
    dt_cur = _dtmax;

  // Don't allow time step size to be smaller than minimum time step size
  if(dt_cur < _dtmin)
    dt_cur = _dtmin;
          
  // Don't let time go beyond simulation end time
  if(_time + dt_cur > _end_time)
    dt_cur = _end_time - _time;
    
  // Increment time
  _time += dt_cur;
  _dt = dt_cur;

  _moose_system.reinitDT();

  std::cout<<"DT: "<<dt_cur<<std::endl;

  std::cout << " Solving time step ";
  {
    OStringStream out;
      
    OSSInt(out,2,_t_step);
    out << ", time=";
    OSSRealzeroleft(out,9,6,_time);
    out <<  "...";
    std::cout << out.str() << std::endl;
  }

  Moose::setSolverDefaults(_moose_system, this);
    
  setScaling();

  preSolve();
    
  Moose::perf_log.push("solve()","Solve");
  // System Solve
  _moose_system.solve();
  Moose::perf_log.pop("solve()","Solve");

  _converged = _moose_system.getNonlinearSystem()->nonlinear_solver->converged;

  postSolve();

  std::cout<<"Norm of each nonlinear variable:"<<std::endl;
  for(unsigned int var = 0; var < _moose_system.getNonlinearSystem()->n_vars(); var++)
    std::cout << _moose_system.getNonlinearSystem()->variable_name(var) << ": "
              << _moose_system.getNonlinearSystem()->calculate_norm(*_moose_system.getNonlinearSystem()->rhs,var,DISCRETE_L2) << std::endl;
    
  if ( _converged && (_t_step+1)%_moose_system._interval == 0)
    _moose_system.output_system(_t_step, _time);

  if( _converged )
  {
    adaptMesh();
            
    _t_step++;
  }
}

Real
TransientExecutioner::computeDT()
{
  if(!lastSolveConverged())
  {
    std::cout<<"Solve failed... cutting timestep"<<std::endl;
    return _dt / 2.0;
  }
  
  // If start up steps are needed
  if(_t_step == 1 && _n_startup_steps > 1)
    return _dt/(double)(_n_startup_steps);
  else
    return _dt;
}

bool
TransientExecutioner::keepGoing()
{
  // Check for stop condition based upon steady-state check flag:
  if(_converged && _trans_ss_check == true && _time > _ss_tmin)
  {
    // Compute new time solution l2_norm
    Real new_time_solution_norm = _moose_system.getNonlinearSystem()->current_local_solution->l2_norm();
          
    // Compute l2_norm relative error
    Real ss_relerr_norm = fabs(new_time_solution_norm - _old_time_solution_norm)/new_time_solution_norm;

    // Check current solution relative error norm against steady-state tolerance
    if(ss_relerr_norm < _ss_check_tol)
    {
      std::cout<<"Steady-State Solution Achieved at time: "<<_time<<std::endl;
      return false;
    }
    else // Keep going
    {
      // Update solution norm for next time step  
      _old_time_solution_norm = new_time_solution_norm;
      // Print steady-state relative error norm
      std::cout<<"Steady-State Relative Differential Norm: "<<ss_relerr_norm<<std::endl;
    }
  }
        
  // Check for stop condition based upon number of simulation steps and/or solution end time:
  if(_t_step>_num_steps)
    return false;
  
  if((_time>_end_time) || (fabs(_time-_end_time)<1.e-14))
    return false;

  return true;
}


bool
TransientExecutioner::lastSolveConverged()
{
  return _converged;
}  
