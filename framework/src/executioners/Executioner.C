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
#include "MooseMesh.h"
#include "FEProblem.h"
#include "MooseApp.h"

// C++ includes
#include <vector>
#include <limits>

template<>
InputParameters validParams<Executioner>()
{
  InputParameters params = validParams<MooseObject>();
  params.addParam<FileNameNoExtension>("restart_file_base", "", "File base name used for restart");

  params.registerBase("Executioner");

  params.addParamNamesToGroup("restart_file_base", "Restart");

  params.addParam<std::vector<std::string> >("splitting", "Top-level splitting defining a hierarchical decomposition into subsystems to help the solver.");

#ifdef LIBMESH_HAVE_PETSC
  params += commonExecutionParameters();
#endif //LIBMESH_HAVE_PETSC

  return params;
}

InputParameters commonExecutionParameters()
{
  InputParameters params = emptyInputParameters();
  MooseEnum solve_type("PJFNK JFNK NEWTON FD LINEAR");
  params.addParam<MooseEnum>   ("solve_type",      solve_type,
                                "PJFNK: Preconditioned Jacobian-Free Newton Krylov "
                                "JFNK: Jacobian-Free Newton Krylov "
                                "NEWTON: Full Newton Solve "
                                "FD: Use finite differences to compute Jacobian "
                                "LINEAR: Solving a linear problem");

  // Line Search Options
#ifdef LIBMESH_HAVE_PETSC
#if PETSC_VERSION_LESS_THAN(3,3,0)
  MooseEnum line_search("default cubic quadratic none basic basicnonorms", "default");
#else
  MooseEnum line_search("default shell none basic l2 bt cp", "default");
#endif
  std::string addtl_doc_str(" (Note: none = basic)");
#else
  MooseEnum line_search("default", "default");
  std::string addtl_doc_str("");
#endif
  params.addParam<MooseEnum>   ("line_search",     line_search, "Specifies the line search type" + addtl_doc_str);

#ifdef LIBMESH_HAVE_PETSC
  MultiMooseEnum common_petsc_options("", "", true);

  params.addParam<MultiMooseEnum>("petsc_options", common_petsc_options, "Singleton PETSc options");
  params.addParam<std::vector<std::string> >("petsc_options_iname", "Names of PETSc name/value pairs");
  params.addParam<std::vector<std::string> >("petsc_options_value", "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\"");
#endif //LIBMESH_HAVE_PETSC
  return params;
}

Executioner::Executioner(const InputParameters & parameters) :
    MooseObject(parameters),
    UserObjectInterface(parameters),
    PostprocessorInterface(parameters),
    Restartable(parameters, "Executioners"),
    _initial_residual_norm(std::numeric_limits<Real>::max()),
    _old_initial_residual_norm(std::numeric_limits<Real>::max()),
    _restart_file_base(getParam<FileNameNoExtension>("restart_file_base")),
    _splitting(getParam<std::vector<std::string> >("splitting"))
{
}

Executioner::~Executioner()
{
}

void
Executioner::init()
{
}

void
Executioner::preExecute()
{
}

void
Executioner::postExecute()
{
}

void
Executioner::preSolve()
{
}

void
Executioner::postSolve()
{
}

std::string
Executioner::getTimeStepperName()
{
  return std::string();
}


void
Executioner::addAttributeReporter(const std::string & name, Real & attribute, const std::string execute_on)
{
  FEProblem * problem = parameters().getCheckedPointerParam<FEProblem *>("_fe_problem", "Failed to retrieve FEProblem when adding a attribute reporter in Executioner");
  InputParameters params = _app.getFactory().getValidParams("ExecutionerAttributeReporter");
  params.set<Real *>("value") = &attribute;
  if (!execute_on.empty())
    params.set<MultiMooseEnum>("execute_on") = execute_on;
  problem->addPostprocessor("ExecutionerAttributeReporter", name, params);
}


// DEPRECATED CONSTRUCTOR
Executioner::Executioner(const std::string & deprecated_name, InputParameters parameters) :
    MooseObject(deprecated_name, parameters),
    UserObjectInterface(parameters),
    PostprocessorInterface(parameters),
    Restartable(parameters, "Executioners"),
    _initial_residual_norm(std::numeric_limits<Real>::max()),
    _old_initial_residual_norm(std::numeric_limits<Real>::max()),
    _restart_file_base(getParam<FileNameNoExtension>("restart_file_base")),
    _splitting(getParam<std::vector<std::string> >("splitting"))
{
}
