#include "ExecutionBlock.h"
#include "Moose.h"

#ifdef LIBMESH_HAVE_PETSC
/*#include "sparse_matrix.h"
#include "petsc_vector.h"
#include "petsc_matrix.h"
#include "petsc_linear_solver.h"
#include "petsc_preconditioner.h"
*/
#include "PetscSupport.h"
#endif //LIBMESH_HAVE_PETSC


ExecutionBlock::ExecutionBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params)
{
  // Register the execution Prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("AuxVariables");
  addPrereq("Kernels");
  addPrereq("AuxKernels");
  addPrereq("BCs");
  addPrereq("AuxBCs");
  addPrereq("Materials");
}

void
ExecutionBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the ExecutionBlock Object\n";
#endif

  Moose::equation_system->parameters.set<Real> ("linear solver tolerance")
    = getParamValue<Real>("l_tol");

  Moose::equation_system->parameters.set<Real> ("linear solver absolute step tolerance")
    = getParamValue<Real>("l_abs_step_tol");

  Moose::equation_system->parameters.set<unsigned int> ("linear solver maximum iterations")
    = getParamValue<unsigned int>("l_max_its");    
  
  Moose::equation_system->parameters.set<unsigned int> ("nonlinear solver maximum iterations")
    = getParamValue<unsigned int>("nl_max_its");
    
  Moose::equation_system->parameters.set<unsigned int> ("nonlinear solver maximum function evaluations")
    = getParamValue<unsigned int>("nl_max_funcs");
    
  Moose::equation_system->parameters.set<Real> ("nonlinear solver absolute residual tolerance")
    = getParamValue<Real>("nl_abs_tol");
  
  Moose::equation_system->parameters.set<Real> ("nonlinear solver relative residual tolerance")
    = getParamValue<Real>("nl_rel_tol");
  
  Moose::equation_system->parameters.set<Real> ("nonlinear solver absolute step tolerance")
    = getParamValue<Real>("nl_abs_step_tol");
  
  Moose::equation_system->parameters.set<Real> ("nonlinear solver relative step tolerance")
    = getParamValue<Real>("nl_rel_step_tol");

  Moose::no_fe_reinit = getParamValue<bool>("no_fe_reinit");

  if (!getParamValue<bool>("perf_log"))
    Moose::perf_log.disable_logging();

  Moose::execution_type = getParamValue<std::string>("type");

  Moose::auto_scaling = getParamValue<bool>("auto_scaling");


#ifdef LIBMESH_HAVE_PETSC
  std::vector<std::string> petsc_options,  petsc_inames, petsc_values;
  petsc_options = getParamValue<std::vector<std::string> >("petsc_options");
  petsc_inames = getParamValue<std::vector<std::string> >("petsc_options_iname");
  petsc_values = getParamValue<std::vector<std::string> >("petsc_options_value");

  MoosePetscSupport::l_abs_step_tol = getParamValue<Real>("l_abs_step_tol");
  
  if (petsc_inames.size() != petsc_values.size())
    mooseError("Petsc names and options from input file are not the same length");

  for (unsigned int i=0; i<petsc_options.size(); ++i)
    PetscOptionsSetValue(petsc_options[i].c_str(), PETSC_NULL);
  
  for (unsigned int i=0; i<petsc_inames.size(); ++i)
    PetscOptionsSetValue(petsc_inames[i].c_str(), petsc_values[i].c_str());
#endif //LIBMESH_HAVE_PETSC
  

  visitChildren();
}
