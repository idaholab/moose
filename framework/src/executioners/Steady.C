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

template<>
InputParameters validParams<Steady>()
{
  InputParameters params = validParams<Executioner>();
  params.addParam<unsigned int>("max_r_steps", 0, "Number of refinement steps to do.");
  return params;
}


Steady::Steady(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :Executioner(name, moose_system, parameters),
   _moose_system(moose_system),
   _max_r_steps(getParam<unsigned int>("max_r_steps")),
   _t_step(moose_system.parameters().set<int> ("t_step") = 0)
{}

void
Steady::execute()
{
  // Define the refinement loop
  for(unsigned int r_step=0; r_step<=_max_r_steps; r_step++)
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
        
    _moose_system.output_system(r_step+1, r_step+1);
    
    for(unsigned int var = 0; var < _moose_system.getNonlinearSystem()->n_vars(); var++)
      std::cout<<var<<": "<<_moose_system.getNonlinearSystem()->calculate_norm(*_moose_system.getNonlinearSystem()->rhs,var,DISCRETE_L2)<<std::endl;
        
    if(r_step != _max_r_steps)
      adaptMesh();
  }
}

bool
Steady::lastSolveConverged()
{
  return true;
}  
