#include "Executioner.h"

// Moose includes
#include "Moose.h"
#include "MooseSystem.h"
#include "Kernel.h"

// C++ includes
#include <vector>
#include <limits>

// libMesh includes
#include "nonlinear_implicit_system.h"
#include "transient_system.h"
#include "kelly_error_estimator.h"
#include "mesh_refinement.h"
#include "numeric_vector.h"

// FIXME: remove me when libmesh solver problem is fixed
namespace Moose {
void compute_residual (const NumericVector<Number>& soln, NumericVector<Number>& residual);
void compute_jacobian (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian);
void compute_jacobian_block (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, unsigned int ivar, unsigned int jvar);
}

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
//   _system(*moose_system.getNonlinearSystem()),
//   _aux_system(*moose_system.getAuxSystem()),
   _initial_residual_norm(std::numeric_limits<Real>::max()),
   _old_initial_residual_norm(std::numeric_limits<Real>::max())
{}

void
Executioner::setup()
{
  // FIXME: !!!
  Moose::setSolverDefaults(_moose_system.getEquationSystems(), *_moose_system.getNonlinearSystem(),
                           Moose::compute_jacobian_block, Moose::compute_residual);

  _moose_system.getNonlinearSystem()->update();

//  _moose_system.copy_old_solutions();
  // Update backward time solution vectors.
  *_moose_system._system->older_local_solution = *_moose_system._system->current_local_solution;
  *_moose_system._system->old_local_solution   = *_moose_system._system->current_local_solution;
  *_moose_system._aux_system->older_local_solution = *_moose_system._aux_system->current_local_solution;
  *_moose_system._aux_system->old_local_solution   = *_moose_system._aux_system->current_local_solution;

  unsigned int initial_adaptivity = 0;

  if(_moose_system.getEquationSystems()->parameters.have_parameter<unsigned int>("initial_adaptivity"))
    initial_adaptivity = _moose_system.getEquationSystems()->parameters.get<unsigned int>("initial_adaptivity");

  //Initial adaptivity
  for(unsigned int i=0; i<initial_adaptivity; i++)
  {
    // Compute the error for each active element
    Moose::error_estimator->estimate_error(*_moose_system.getNonlinearSystem(), *Moose::error);
      
    // Flag elements to be refined and coarsened
    Moose::mesh_refinement->flag_elements_by_error_fraction (*Moose::error);
          
    // Perform refinement and coarsening
    Moose::mesh_refinement->refine_and_coarsen_elements();

    // Tell MOOSE that the mesh has changed
    // this performs a lot of functions including projecting
    // the solution onto the new grid.
    _moose_system.meshChanged();

    //reproject the initial condition
    _moose_system.getNonlinearSystem()->project_solution(Moose::init_value, NULL, _moose_system.getEquationSystems()->parameters);
  }    

  if(Moose::output_initial)
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
  if(Moose::mesh_refinement)
  {
    // Compute the error for each active element
    Moose::error_estimator->estimate_error(*_moose_system.getNonlinearSystem(), *Moose::error);
          
    // Flag elements to be refined and coarsened
    Moose::mesh_refinement->flag_elements_by_error_fraction (*Moose::error);
          
    // Perform refinement and coarsening
    Moose::mesh_refinement->refine_and_coarsen_elements();
          
    // Tell MOOSE that the Mesh has changed
    _moose_system.meshChanged();
  }
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
  _moose_system.setVarScaling(Moose::manual_scaling);

  if(Moose::auto_scaling)
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

