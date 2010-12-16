/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "Steady.h"
#include "MooseSystem.h"
#include "equation_systems.h" // libMesh

template<>
InputParameters validParams<Steady>()
{
  InputParameters params = validParams<Executioner>();
  params.addParam<unsigned int>("max_r_steps", 0, "DEPRECATED - Please put a parameter named \"steps\" inside of the adaptivity block");
  return params;
}


Steady::Steady(const std::string & name, InputParameters parameters)
  :Executioner(name, parameters),
   // This parameter is shoved in from the child adaptivity block by the GenericExecutionerBlock
   _steps(getParam<unsigned int>("steps")),
   _t_step(_moose_system.parameters().set<int> ("t_step") = 0)
{}

void
Steady::execute()
{
  // Define the refinement loop
  for(unsigned int r_step=0; r_step<=_steps; r_step++)
  {
    _t_step = r_step+1;

    _moose_system._t_step = _t_step;
    
    _moose_system.getNonlinearSystem()->print_info();

    setScaling();

    preSolve();

    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
      _moose_system._materials[tid].updateMaterialDataState();
    
//    PerfLog solve_only("Solve Only");
//    solve_only.push("solve()","Solve");

    Moose::perf_log.push("solve()","Solve");

    _moose_system.solve();

    Moose::perf_log.pop("solve()","Solve");

//    solve_only.pop("solve()","Solve");

    postSolve();

    // We know whether or not the nonlinear solver thinks it converged, but we need to see if the executioner concurs
    bool last_solve_converged = lastSolveConverged();

    if (last_solve_converged) 
    {
      _moose_system.computePostprocessors(*(_moose_system.getNonlinearSystem()->current_local_solution));
      _moose_system.outputPostprocessors();
      _moose_system.outputSystem(r_step+1, r_step+1);
    }
    
    for(unsigned int var = 0; var < _moose_system.getNonlinearSystem()->n_vars(); var++)
      std::cout<<var<<": "<<_moose_system.getNonlinearSystem()->calculate_norm(*_moose_system.getNonlinearSystem()->rhs,var,DISCRETE_L2)<<std::endl;
        
    if(r_step != _steps)
      adaptMesh();
  }
}

bool
Steady::lastSolveConverged()
{
  return true;
}  
