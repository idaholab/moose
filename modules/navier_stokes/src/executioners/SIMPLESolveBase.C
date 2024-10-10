//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimpleSolveBase.h"
#include "FEProblem.h"
#include "SegregatedSolverUtils.h"

InputParameters
SimpleSolveBase::validParams()
{
  InputParameters params = emptyInputParameters();

  /*
   * The names of the different systems in the segregated solver
   */
  params.addRequiredParam<std::vector<SolverSystemName>>(
      "momentum_systems", "The solver system(s) for the momentum equation(s).");
  params.addRequiredParam<SolverSystemName>("pressure_system",
                                            "The solver system for the pressure equation.");

  /*
   * Parameters to control the solution of the momentum equation
   */

  params.addRangeCheckedParam<Real>(
      "momentum_equation_relaxation",
      1.0,
      "0.0<momentum_equation_relaxation<=1.0",
      "The relaxation which should be used for the momentum equation. (=1 for no relaxation, "
      "diagonal dominance will still be enforced)");

  params.addParam<MultiMooseEnum>("momentum_petsc_options",
                                  Moose::PetscSupport::getCommonPetscFlags(),
                                  "Singleton PETSc options for the momentum equation");
  params.addParam<MultiMooseEnum>("momentum_petsc_options_iname",
                                  Moose::PetscSupport::getCommonPetscKeys(),
                                  "Names of PETSc name/value pairs for the momentum equation");
  params.addParam<std::vector<std::string>>(
      "momentum_petsc_options_value",
      "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\" for the "
      "momentum equation");

  params.addRangeCheckedParam<Real>(
      "momentum_absolute_tolerance",
      1e-5,
      "0.0<momentum_absolute_tolerance",
      "The absolute tolerance on the normalized residual of the momentum equation.");

  params.addRangeCheckedParam<Real>("momentum_l_tol",
                                    1e-5,
                                    "0.0<=momentum_l_tol & momentum_l_tol<1.0",
                                    "The relative tolerance on the normalized residual in the "
                                    "linear solver of the momentum equation.");
  params.addRangeCheckedParam<Real>("momentum_l_abs_tol",
                                    1e-50,
                                    "0.0<momentum_l_abs_tol",
                                    "The absolute tolerance on the normalized residual in the "
                                    "linear solver of the momentum equation.");
  params.addParam<unsigned int>(
      "momentum_l_max_its",
      10000,
      "The maximum allowed iterations in the linear solver of the momentum equation.");

  params.addParamNamesToGroup(
      "momentum_equation_relaxation momentum_petsc_options momentum_petsc_options_iname "
      "momentum_petsc_options_value momentum_petsc_options_value momentum_absolute_tolerance "
      "momentum_l_tol momentum_l_abs_tol momentum_l_max_its",
      "Momentum Equation");

  /*
   * Parameters to control the solution of the pressure equation
   */
  params.addRangeCheckedParam<Real>(
      "pressure_variable_relaxation",
      1.0,
      "0.0<pressure_variable_relaxation<=1.0",
      "The relaxation which should be used for the pressure variable (=1 for no relaxation).");

  params.addParam<MultiMooseEnum>("pressure_petsc_options",
                                  Moose::PetscSupport::getCommonPetscFlags(),
                                  "Singleton PETSc options for the pressure equation");
  params.addParam<MultiMooseEnum>("pressure_petsc_options_iname",
                                  Moose::PetscSupport::getCommonPetscKeys(),
                                  "Names of PETSc name/value pairs for the pressure equation");
  params.addParam<std::vector<std::string>>(
      "pressure_petsc_options_value",
      "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\" for the "
      "pressure equation");

  params.addRangeCheckedParam<Real>(
      "pressure_absolute_tolerance",
      1e-5,
      "0.0<pressure_absolute_tolerance",
      "The absolute tolerance on the normalized residual of the pressure equation.");

  params.addRangeCheckedParam<Real>("pressure_l_tol",
                                    1e-5,
                                    "0.0<=pressure_l_tol & pressure_l_tol<1.0",
                                    "The relative tolerance on the normalized residual in the "
                                    "linear solver of the pressure equation.");
  params.addRangeCheckedParam<Real>("pressure_l_abs_tol",
                                    1e-10,
                                    "0.0<pressure_l_abs_tol",
                                    "The absolute tolerance on the normalized residual in the "
                                    "linear solver of the pressure equation.");
  params.addParam<unsigned int>(
      "pressure_l_max_its",
      10000,
      "The maximum allowed iterations in the linear solver of the pressure equation.");

  params.addParamNamesToGroup(
      "pressure_variable_relaxation pressure_petsc_options pressure_petsc_options_iname "
      "pressure_petsc_options_value pressure_petsc_options_value pressure_absolute_tolerance "
      "pressure_l_tol pressure_l_abs_tol pressure_l_max_its",
      "Pressure Equation");

  /*
   * Pressure pin parameters for enclosed flows
   */

  params.addParam<bool>(
      "pin_pressure", false, "If the pressure field needs to be pinned at a point.");
  params.addParam<Real>(
      "pressure_pin_value", 0.0, "The value which needs to be enforced for the pressure.");
  params.addParam<Point>("pressure_pin_point", "The point where the pressure needs to be pinned.");

  params.addParamNamesToGroup("pin_pressure pressure_pin_value pressure_pin_point", "Pressure Pin");

  params.addParam<bool>(
      "print_fields",
      false,
      "Use this to print the coupling and solution fields and matrices throughout the iteration.");

  return params;
}

SimpleSolveBase::SimpleSolveBase(Executioner & ex)
  : SolveObject(ex),
    _momentum_system_names(getParam<std::vector<SolverSystemName>>("momentum_systems")),
    _momentum_l_abs_tol(getParam<Real>("momentum_l_abs_tol")),
    _momentum_absolute_tolerance(getParam<Real>("momentum_absolute_tolerance")),
    _momentum_equation_relaxation(getParam<Real>("momentum_equation_relaxation")),
    _pressure_system_name(getParam<SolverSystemName>("pressure_system")),
    _pressure_sys_number(_problem.linearSysNum(_pressure_system_name)),
    _pressure_system(_problem.getLinearSystem(_pressure_sys_number)),
    _pressure_l_abs_tol(getParam<Real>("pressure_l_abs_tol")),
    _pressure_absolute_tolerance(getParam<Real>("pressure_absolute_tolerance")),
    _pressure_variable_relaxation(getParam<Real>("pressure_variable_relaxation")),
    _pin_pressure(getParam<bool>("pin_pressure")),
    _pressure_pin_value(getParam<Real>("pressure_pin_value")),
    _print_fields(getParam<bool>("print_fields"))
{
  const auto & momentum_petsc_options = getParam<MultiMooseEnum>("momentum_petsc_options");
  const auto & momentum_petsc_pair_options = getParam<MooseEnumItem, std::string>(
      "momentum_petsc_options_iname", "momentum_petsc_options_value");
  Moose::PetscSupport::processPetscFlags(momentum_petsc_options, _momentum_petsc_options);
  Moose::PetscSupport::processPetscPairs(
      momentum_petsc_pair_options, _problem.mesh().dimension(), _momentum_petsc_options);

  _momentum_linear_control.real_valued_data["rel_tol"] = getParam<Real>("momentum_l_tol");
  _momentum_linear_control.real_valued_data["abs_tol"] = getParam<Real>("momentum_l_abs_tol");
  _momentum_linear_control.int_valued_data["max_its"] =
      getParam<unsigned int>("momentum_l_max_its");

  const auto & pressure_petsc_options = getParam<MultiMooseEnum>("pressure_petsc_options");
  const auto & pressure_petsc_pair_options = getParam<MooseEnumItem, std::string>(
      "pressure_petsc_options_iname", "pressure_petsc_options_value");
  Moose::PetscSupport::processPetscFlags(pressure_petsc_options, _pressure_petsc_options);
  Moose::PetscSupport::processPetscPairs(
      pressure_petsc_pair_options, _problem.mesh().dimension(), _pressure_petsc_options);

  _pressure_linear_control.real_valued_data["rel_tol"] = getParam<Real>("pressure_l_tol");
  _pressure_linear_control.real_valued_data["abs_tol"] = getParam<Real>("pressure_l_abs_tol");
  _pressure_linear_control.int_valued_data["max_its"] =
      getParam<unsigned int>("pressure_l_max_its");
}

void
SimpleSolveBase::setupPressurePin()
{
  if (_pin_pressure)
    _pressure_pin_dof = NS::FV::findPointDoFID(_problem.getVariable(0, "pressure"),
                                               _problem.mesh(),
                                               getParam<Point>("pressure_pin_point"));
}
