#include "Executioner.h"

// Moose includes
#include "Moose.h"
#include "ComputeJacobian.h"
#include "ComputeResidual.h"
#include "Kernel.h"

// C++ includes
#include <vector>
#include <limits>

// libMesh includes
#include "nonlinear_implicit_system.h"
#include "transient_system.h"
#include "kelly_error_estimator.h"
#include "mesh_refinement.h"

template<>
InputParameters validParams<Executioner>()
{
  InputParameters params;
  return params;
}


Executioner::Executioner(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :_name(name),
   _moose_system(moose_system),
   _parameters(parameters),
   _system(Moose::equation_system->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem")),
   _aux_system(Moose::equation_system->get_system<TransientExplicitSystem>("AuxiliarySystem")),
   _initial_residual_norm(std::numeric_limits<Real>::max()),
   _old_initial_residual_norm(std::numeric_limits<Real>::max())
{}

bool
Executioner::setup()
{
  Moose::setSolverDefaults(Moose::equation_system, _system, Moose::compute_jacobian_block, Moose::compute_residual);

  unsigned int initial_adaptivity = 0;

  if(Moose::equation_system->parameters.have_parameter<unsigned int>("initial_adaptivity"))
    initial_adaptivity = Moose::equation_system->parameters.get<unsigned int>("initial_adaptivity");

  //Initial adaptivity
  for(unsigned int i=0; i<initial_adaptivity; i++)
  {
    // Compute the error for each active element
    Moose::error_estimator->estimate_error(_system, *Moose::error);
      
    // Flag elements to be refined and coarsened
    Moose::mesh_refinement->flag_elements_by_error_fraction (*Moose::error);
          
    // Perform refinement and coarsening
    Moose::mesh_refinement->refine_and_coarsen_elements();

    // Tell MOOSE that the mesh has changed
    // this performs a lot of functions including projecting
    // the solution onto the new grid.
    Moose::meshChanged();

    //reproject the initial condition
    _system.project_solution(Moose::init_value, NULL, Moose::equation_system->parameters);
  }    

  if(Moose::output_initial)
  {
    std::cout<<"Outputting Initial Condition"<<std::endl;
      
    Moose::output_system(0, 0.0);
  }
    
  // Check for Kernel, BC, and Material coverage on the Mesh
  Moose::checkSystemsIntegrity();
}

void
Executioner::adaptMesh()
{
  if(Moose::mesh_refinement)
  {
    // Compute the error for each active element
    Moose::error_estimator->estimate_error(_system, *Moose::error);
          
    // Flag elements to be refined and coarsened
    Moose::mesh_refinement->flag_elements_by_error_fraction (*Moose::error);
          
    // Perform refinement and coarsening
    Moose::mesh_refinement->refine_and_coarsen_elements();
          
    // Tell MOOSE that the Mesh has changed
    Moose::meshChanged();
  }
}

void
Executioner::setScaling()
{
  std::vector<Real> one_scaling;
                  
  // Reset the scaling to all 1's so we can compute the true residual
  for(unsigned int var = 0; var < _system.n_vars(); var++)
    one_scaling.push_back(1.0);

  Kernel::setVarScaling(one_scaling);
  
  Moose::compute_residual(*_system.current_local_solution,*_system.rhs);

  _old_initial_residual_norm = _initial_residual_norm;
  _initial_residual_norm = _system.rhs->l2_norm();
    
  std::cout<<"  True Initial Nonlinear Residual: "<<_initial_residual_norm<<std::endl;
  
  // Set the scaling to manual scaling
  Kernel::setVarScaling(Moose::manual_scaling);

  if(Moose::auto_scaling)
  {
    std::vector<Real> scaling;
      
    // Compute the new scaling
    for(unsigned int var = 0; var < _system.n_vars(); var++)
    {
      Real norm = _system.calculate_norm(*_system.rhs,var,DISCRETE_L2);
      
      if(norm != 0)
        scaling.push_back(1.0/norm);
      else
        scaling.push_back(1.0);
    }
          
    Kernel::setVarScaling(scaling);
  }
}

