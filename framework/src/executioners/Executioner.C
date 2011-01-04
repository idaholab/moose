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

#include "Executioner.h"

// Moose includes
#include "Moose.h"
#include "MooseSystem.h"
#include "Kernel.h"
#include "ComputeJacobian.h"
#include "ComputeResidual.h"
#include "ComputeInitialConditions.h"
#include "PetscSupport.h"

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
  InputParameters params = validParams<MooseObject>();
  return params;
}


Executioner::Executioner(const std::string & name, InputParameters parameters) :
  MooseObject(name, parameters),
  FunctionInterface(parameters.get<MooseSystem *>("_moose_system")->_functions[_tid], parameters),
  _moose_system(*parameters.get<MooseSystem *>("_moose_system")),
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
  Moose::setSolverDefaults(_moose_system, this);

  // Check for Kernel, BC, and Material coverage on the Mesh
  _moose_system.checkSystemsIntegrity();

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
    _moose_system.adaptMesh();

    //_moose_system.doAdaptivityStep();

    //reproject the initial condition
    _moose_system.projectSolution(Moose::initial_value, Moose::initial_gradient);
  }

  // Run the geometric searches for the first time
  _moose_system._geometric_search_data.update();
  _moose_system._geometric_search_data_displaced.update();

  // Compute the initial value of postprocessors
  _moose_system.computePostprocessors(*(_moose_system.getNonlinearSystem()->current_local_solution));
  _moose_system.outputPostprocessors();

  if(_moose_system._output_initial)
  {
    std::cout<<"Outputting Initial Condition"<<std::endl;

    _moose_system.outputSystem(0, 0.0);
  }

  _moose_system.getNonlinearSystem()->update();

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

  _moose_system.computeResidual(*_moose_system.getNonlinearSystem()->current_local_solution,
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

void
Executioner::updateNewtonStep()
{
  _moose_system.updateNewtonStep();
}

void
Executioner::postSolve()
{
  std::cout<<"Post solving!"<<std::endl;
  _moose_system.postSolve(_moose_system.getNonlinearSystem()->nonlinear_solver->converged);
}
