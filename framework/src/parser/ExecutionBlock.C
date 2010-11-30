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

#if 0   // DEPRECATED - USE EXECUTIONER SYSTEM

#include "ExecutionBlock.h"
#include "Moose.h"

#ifdef LIBMESH_HAVE_PETSC
#include "PetscSupport.h"
#endif //LIBMESH_HAVE_PETSC

template<>
InputParameters validParams<ExecutionBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addRequiredParam<std::string> ("type", "Specifies \"Steady\" or \"Transient\" Solver Strategy");
  
  params.addParam<Real>        ("l_tol",           1.0e-5,   "Linear Tolerance");
  params.addParam<Real>        ("l_abs_step_tol",  -1,       "Linear Absolute Step Tolerance");
  params.addParam<unsigned int>("l_max_its",       10000,    "Max Linear Iterations");
  params.addParam<unsigned int>("nl_max_its",      50,       "Max Nonlinear Iterations");
  params.addParam<unsigned int>("nl_max_funcs",    10000,    "Max Nonlinear solver function evaluations");
  params.addParam<Real>        ("nl_abs_tol",      1.0e-50,  "Nonlinear Absolute Tolerance");
  params.addParam<Real>        ("nl_rel_tol",      1.0e-8,   "Nonlinear Relative Tolerance");
  params.addParam<Real>        ("nl_abs_step_tol", 1.0e-50,  "Nonlinear Absolute step Tolerance");
  params.addParam<Real>        ("nl_rel_step_tol", 1.0e-50,  "Nonlinear Relative step Tolerance");
  params.addParam<bool>        ("no_fe_reinit",    false,    "Specifies whether or not to reinitialize FEs");
  params.addParam<bool>        ("perf_log",        false,    "Specifies whether or not the Performance log should be printed");
  params.addParam<bool>        ("auto_scaling",    false,    "Turns on automatic variable scaling");

  params.addParam<std::string> ("solve_type", "JFNK", "Tells the solver which nonlinear solution method to use (JFNK, PJFNK, NEWTON)");
  
#ifdef LIBMESH_HAVE_PETSC
  params.addParam<std::vector<std::string> >("petsc_options", "Singleton Petsc options (Consider using the preconditioner optiion)");
  params.addParam<std::vector<std::string> >("petsc_options_iname", "Names of Petsc name/value pairs");
  params.addParam<std::vector<std::string> >("petsc_options_value", "Values of Petsc name/value pairs (must correspond with \"petsc_options_iname\"");

  params.addParam<std::string>("preconditioner", "Specifies the preconditioner to use (AMG | ILU | LU)");
  params.addParam<unsigned int>("gmres_restart", "Specifies the number of iteratations values that GMRES holds");
  params.addParam<bool>("eisenstat_walker", false, "Specifies that the Eisenstat-Walker method is used to determine linear convergence");
#endif //LIBMESH_HAVE_PETSC

  return params;
}

ExecutionBlock::ExecutionBlock(const std::string & name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{
  // Register Execution prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("Preconditioning");
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

  EquationSystems *es = _moose_system.getEquationSystems();
  es->parameters.set<Real> ("linear solver tolerance") = getParamValue<Real>("l_tol");

  es->parameters.set<Real> ("linear solver absolute step tolerance") = getParamValue<Real>("l_abs_step_tol");

  es->parameters.set<unsigned int> ("linear solver maximum iterations") = getParamValue<unsigned int>("l_max_its");
  
  es->parameters.set<unsigned int> ("nonlinear solver maximum iterations") = getParamValue<unsigned int>("nl_max_its");
    
  es->parameters.set<unsigned int> ("nonlinear solver maximum function evaluations") = getParamValue<unsigned int>("nl_max_funcs");
    
  es->parameters.set<Real> ("nonlinear solver absolute residual tolerance") = getParamValue<Real>("nl_abs_tol");
  
  es->parameters.set<Real> ("nonlinear solver relative residual tolerance") = getParamValue<Real>("nl_rel_tol");
  
  es->parameters.set<Real> ("nonlinear solver absolute step tolerance") = getParamValue<Real>("nl_abs_step_tol");
  
  es->parameters.set<Real> ("nonlinear solver relative step tolerance") = getParamValue<Real>("nl_rel_step_tol");

  _moose_system.dontReinitFE() = getParamValue<bool>("no_fe_reinit");

  if (!getParamValue<bool>("perf_log"))
    Moose::perf_log.disable_logging();

  Moose::execution_type = getParamValue<std::string>("type");

  Moose::auto_scaling = getParamValue<bool>("auto_scaling");


#ifdef LIBMESH_HAVE_PETSC
  std::vector<std::string> petsc_options,  petsc_inames, petsc_values;
  petsc_options = getParamValue<std::vector<std::string> >("petsc_options");
  petsc_inames = getParamValue<std::vector<std::string> >("petsc_options_iname");
  petsc_values = getParamValue<std::vector<std::string> >("petsc_options_value");

  switch (getParamValue<std::string>("solve_type"))
  {
  case "JFNK":
    petsc_options += ' -snes_mf';
    break;
  case "PJFNK":
    petsc_options += ' -snes_mf_operator';
    break;
  case "NEWTON":
    petsc_options += ' -snes';
    mooseWarning("The Newton Method only works in limited multiphysics settings");
    break;
  }  
  
  // Using the new preconditioner options supersede the older petsc options
  if (isParamValid<std::string>("preconditioner"))
  {
    if (petsc_options.find("-snes_mf_operator") == std::string::npos)
      mooseWarning("A Preconditioner has been set but Preconditioned JFNK is not being used");
    switch (getParamValue<std::string>("preconditioner"))
    {
    case "AMG":
      petsc_inames = "-pc_type -pc_hypre_type";
      petsc_values = "hypre boomeramg";
      break;
    case "ILU":
      petsc_inames = "-pc_type";
      petsc_values = "ilu";
      break;
    case "LU":
      petsc_inames = "-pc_type";
      petsc_values = "lu";
      break;
    default:
      mooseError("Unsupported preconditioner option detected, Please check your ""preconditioner"" option in your Executioner Block");
    }

  if (getParamValue<bool>("eisenstat_walker"))
    PetscOptionsSetValue("-snes_ksp_ew", PETSC_NULL);

    

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

#endif
