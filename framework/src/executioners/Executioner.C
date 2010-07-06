#include "Executioner.h"

// Moose includes
#include "Moose.h"
#include "MooseSystem.h"
#include "Kernel.h"
#include "ComputeJacobian.h"
#include "ComputeResidual.h"
#include "ComputeInitialConditions.h"

// C++ includes
#include <vector>
#include <limits>

// libMesh includes
#include "nonlinear_implicit_system.h"
#include "transient_system.h"
#include "kelly_error_estimator.h"
#include "mesh_refinement.h"
#include "numeric_vector.h"

template<>
InputParameters validParams<Executioner>()
{
  InputParameters params;
  return params;
}


Executioner::Executioner(std::string name, MooseSystem & moose_system, InputParameters parameters) :
  MooseObject(name, moose_system, parameters),
  _initial_residual_norm(std::numeric_limits<Real>::max()),
  _old_initial_residual_norm(std::numeric_limits<Real>::max())
{
}

Executioner::~Executioner()
{
}

void
Executioner::setup()
{
  // FIXME: !!!
  Moose::setSolverDefaults(_moose_system.getEquationSystems(), *_moose_system.getNonlinearSystem(),
                           Moose::compute_jacobian_block, Moose::compute_residual);

  _moose_system.getNonlinearSystem()->update();

  // Update backward time solution vectors.
  *_moose_system._system->older_local_solution = *_moose_system._system->current_local_solution;
  *_moose_system._system->old_local_solution   = *_moose_system._system->current_local_solution;
  *_moose_system._aux_system->older_local_solution = *_moose_system._aux_system->current_local_solution;
  *_moose_system._aux_system->old_local_solution   = *_moose_system._aux_system->current_local_solution;

  unsigned int initial_adaptivity = _moose_system.getInitialAdaptivityStepCount();

  //Initial adaptivity
  for(unsigned int i=0; i<initial_adaptivity; i++)
  {
    _moose_system.doAdaptivityStep();

    // Tell MOOSE that the mesh has changed
    // this performs a lot of functions including projecting
    // the solution onto the new grid.
    _moose_system.meshChanged();

    //reproject the initial condition
    _moose_system.project_solution(Moose::initial_value, NULL);
  }    

  if(_moose_system._output_initial)
  {
    std::cout<<"Outputting Initial Condition"<<std::endl;
      
    _moose_system.output_system(0, 0.0);
  }
    
  // Check for Kernel, BC, and Material coverage on the Mesh
  _moose_system.checkSystemsIntegrity();
}

void
Executioner::adaptMesh()
{
  _moose_system.adaptMesh();
}

void
Executioner::setScaling()
{
  std::vector<Real> one_scaling;
                  
  // Reset the scaling to all 1's so we can compute the true residual
  for(unsigned int var = 0; var < _moose_system.getNonlinearSystem()->n_vars(); var++)
    one_scaling.push_back(1.0);

  _moose_system.setVarScaling(one_scaling);
  
  _moose_system.compute_residual(*_moose_system.getNonlinearSystem()->current_local_solution,
                                 *_moose_system.getNonlinearSystem()->rhs);

  _old_initial_residual_norm = _initial_residual_norm;
  _initial_residual_norm = _moose_system.getNonlinearSystem()->rhs->l2_norm();
    
  std::cout<<"  True Initial Nonlinear Residual: "<<_initial_residual_norm<<std::endl;
  
  // Set the scaling to manual scaling
  _moose_system.setVarScaling(_moose_system._manual_scaling);

  if (_moose_system._auto_scaling)
  {
    std::vector<Real> scaling;
      
    // Compute the new scaling
    for(unsigned int var = 0; var < _moose_system.getNonlinearSystem()->n_vars(); var++)
    {
      Real norm = _moose_system.getNonlinearSystem()->calculate_norm(*_moose_system.getNonlinearSystem()->rhs,var,DISCRETE_L2);
      
      if(norm != 0)
        scaling.push_back(1.0/norm);
      else
        scaling.push_back(1.0);
    }
          
    _moose_system.setVarScaling(scaling);
  }
}

