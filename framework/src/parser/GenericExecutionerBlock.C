/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "GenericExecutionerBlock.h"
#include "ExecutionerFactory.h"
#include "PetscSupport.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericExecutionerBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addRequiredParam<std::string> ("type", "Specifies the type of Executioner to be used");

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

#ifdef LIBMESH_HAVE_PETSC
  params.addParam<std::vector<std::string> >("petsc_options", "Singleton Petsc options");
  params.addParam<std::vector<std::string> >("petsc_options_iname", "Names of Petsc name/value pairs");
  params.addParam<std::vector<std::string> >("petsc_options_value", "Values of Petsc name/value pairs (must correspond with \"petsc_options_iname\"");
#endif //LIBMESH_HAVE_PETSC

  return params;
}

GenericExecutionerBlock::GenericExecutionerBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params),
   _type(getType())
{
  // Register execution prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("AuxVariables");  
  
  setClassParams(ExecutionerFactory::instance()->getValidParams(_type));
}

void
GenericExecutionerBlock::execute() 
{
  InputParameters classParams = getClassParams();
  classParams.set<THREAD_ID>("_tid") = 0;            // have to set '_tid'

  _parser_handle._executioner = ExecutionerFactory::instance()->build(_type, "Executioner", _parser_handle.getMooseSystem(), classParams);

  EquationSystems *es = _moose_system.getEquationSystems();
  es->parameters.set<Real> ("linear solver tolerance")
    = getParamValue<Real>("l_tol");

  es->parameters.set<Real> ("linear solver absolute step tolerance")
    = getParamValue<Real>("l_abs_step_tol");

  es->parameters.set<unsigned int> ("linear solver maximum iterations")
    = getParamValue<unsigned int>("l_max_its");    
  
  es->parameters.set<unsigned int> ("nonlinear solver maximum iterations")
    = getParamValue<unsigned int>("nl_max_its");
    
  es->parameters.set<unsigned int> ("nonlinear solver maximum function evaluations")
    = getParamValue<unsigned int>("nl_max_funcs");
    
  es->parameters.set<Real> ("nonlinear solver absolute residual tolerance")
    = getParamValue<Real>("nl_abs_tol");
  
  es->parameters.set<Real> ("nonlinear solver relative residual tolerance")
    = getParamValue<Real>("nl_rel_tol");
  
  es->parameters.set<Real> ("nonlinear solver absolute step tolerance")
    = getParamValue<Real>("nl_abs_step_tol");
  
  es->parameters.set<Real> ("nonlinear solver relative step tolerance")
    = getParamValue<Real>("nl_rel_step_tol");

  _moose_system._no_fe_reinit = getParamValue<bool>("no_fe_reinit");

  if (!getParamValue<bool>("perf_log"))
    Moose::perf_log.disable_logging();

  _moose_system._auto_scaling = getParamValue<bool>("auto_scaling");


#ifdef LIBMESH_HAVE_PETSC
  std::vector<std::string> petsc_options,  petsc_inames, petsc_values;
  petsc_options = getParamValue<std::vector<std::string> >("petsc_options");
  petsc_inames = getParamValue<std::vector<std::string> >("petsc_options_iname");
  petsc_values = getParamValue<std::vector<std::string> >("petsc_options_value");

  _moose_system._l_abs_step_tol = getParamValue<Real>("l_abs_step_tol");
  
  if (petsc_inames.size() != petsc_values.size())
    mooseError("Petsc names and options from input file are not the same length");

  for (unsigned int i=0; i<petsc_options.size(); ++i)
    PetscOptionsSetValue(petsc_options[i].c_str(), PETSC_NULL);
  
  for (unsigned int i=0; i<petsc_inames.size(); ++i)
    PetscOptionsSetValue(petsc_inames[i].c_str(), petsc_values[i].c_str());
#endif //LIBMESH_HAVE_PETSC

  visitChildren();
}
