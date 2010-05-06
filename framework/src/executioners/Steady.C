#include "Steady.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<Steady>()
{
  InputParameters params = validParams<Executioner>();
  params.addParam<int>("max_r_steps", 0, "Number of refinement steps to do.");
  return params;
}


Steady::Steady(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Executioner(name, moose_system, parameters),
   _max_r_steps(parameters.get<int>("max_r_steps"))
{}

void
Steady::execute()
{
  // Define the refinement loop
  for(unsigned int r_step=0; r_step<=_max_r_steps; r_step++)
  {
    _moose_system.getNonlinearSystem()->print_info();

    setScaling();

    preSolve();
    
    PerfLog solve_only("Solve Only");
    solve_only.push("solve()","Solve");

    Moose::perf_log.push("solve()","Solve");

    _moose_system.solve();

    Moose::perf_log.pop("solve()","Solve");
        
    solve_only.pop("solve()","Solve");

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
