#include "ExecutionBlock.h"

#ifdef LIBMESH_HAVE_PETSC
/*#include "sparse_matrix.h"
#include "petsc_vector.h"
#include "petsc_matrix.h"
#include "petsc_linear_solver.h"
#include "petsc_preconditioner.h"
*/
#include "PetscSupport.h"
#endif //LIBMESH_HAVE_PETSC


ExecutionBlock::ExecutionBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle)
  :ParserBlock(reg_id, real_id, parent, parser_handle)
{
  addParam<std::string> ("type",            "",       "Specifies \"Steady\" or \"Transient\" Solver Strategy", true);
  addParam<Real>        ("l_tol",           1.0e-5,   "Linear Tolerance",                  false);
  addParam<Real>        ("l_abs_step_tol",  -1,       "Linear Absolute Step Tolerance",    false);
  addParam<unsigned int>("l_max_its",       10000,    "Max Linear Iterations",             false);
  addParam<unsigned int>("nl_max_its",      50,       "Max Nonlinear Iterations",          false);
  addParam<unsigned int>("nl_max_funcs",    10000,    "Max Nonlinear solver function evaluations", false);
  addParam<Real>        ("nl_abs_tol",      1.0e-50,  "Nonlinear Absolute Tolerance",      false);
  addParam<Real>        ("nl_rel_tol",      1.0e-8,  "Nonlinear Relative Tolerance",      false);
  addParam<Real>        ("nl_abs_step_tol", 1.0e-50,  "Nonlinear Absolute step Tolerance", false);
  addParam<Real>        ("nl_rel_step_tol", 1.0e-50,  "Nonlinear Relative step Tolerance", false);
  addParam<bool>        ("no_fe_reinit",    false,    "Specifies whether or not to reinitialize FEs", false);
  addParam<bool>        ("perf_log",        false,    "Specifies whether or not the Performance log should be printed", false);
  addParam<bool>        ("auto_scaling",    false,    "Turns on automatic variable scaling", false);

#ifdef LIBMESH_HAVE_PETSC
  addParam<std::vector<std::string> >("petsc_options", "Singleton Petsc options", false);
  addParam<std::vector<std::string> >("petsc_options_iname", "Names of Petsc name/value pairs", false);
  addParam<std::vector<std::string> >("petsc_options_value", "Values of Petsc name/value pairs (must correspond with \"petsc_options_iname\"", false);
#endif //LIBMESH_HAVE_PETSC
  
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
