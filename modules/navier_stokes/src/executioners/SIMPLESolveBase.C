//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SIMPLESolveBase.h"
#include "FEProblem.h"
#include "SegregatedSolverUtils.h"

InputParameters
SIMPLESolveBase::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addRequiredParam<UserObjectName>("rhie_chow_user_object", "The rhie-chow user-object");

  /*
   * The names of the different systems in the segregated solver
   */
  params.addRequiredParam<std::vector<SolverSystemName>>(
      "momentum_systems", "The solver system(s) for the momentum equation(s).");
  params.addRequiredParam<SolverSystemName>("pressure_system",
                                            "The solver system for the pressure equation.");
  params.addParam<SolverSystemName>("energy_system", "The solver system for the energy equation.");
  params.addParam<SolverSystemName>("solid_energy_system",
                                    "The solver system for the solid energy equation.");
  params.addParam<std::vector<SolverSystemName>>(
      "passive_scalar_systems", {}, "The solver system for each scalar advection equation.");
  params.addParam<std::vector<SolverSystemName>>(
      "turbulence_systems", {}, "The solver system for each surrogate turbulence equation.");

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
      "momentum_l_tol momentum_l_abs_tol momentum_l_max_its momentum_systems",
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
      "pressure_l_tol pressure_l_abs_tol pressure_l_max_its pressure_system",
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

  /*
   * Parameters to control the solution of the energy equation
   */

  params.addRangeCheckedParam<Real>(
      "energy_equation_relaxation",
      1.0,
      "0.0<energy_equation_relaxation<=1.0",
      "The relaxation which should be used for the energy equation. (=1 for no relaxation, "
      "diagonal dominance will still be enforced)");

  params.addParam<MultiMooseEnum>("energy_petsc_options",
                                  Moose::PetscSupport::getCommonPetscFlags(),
                                  "Singleton PETSc options for the energy equation");
  params.addParam<MultiMooseEnum>("energy_petsc_options_iname",
                                  Moose::PetscSupport::getCommonPetscKeys(),
                                  "Names of PETSc name/value pairs for the energy equation");
  params.addParam<std::vector<std::string>>(
      "energy_petsc_options_value",
      "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\" for the "
      "energy equation");

  params.addRangeCheckedParam<Real>(
      "energy_absolute_tolerance",
      1e-5,
      "0.0<energy_absolute_tolerance",
      "The absolute tolerance on the normalized residual of the energy equation.");

  params.addRangeCheckedParam<Real>("energy_l_tol",
                                    1e-5,
                                    "0.0<=energy_l_tol & energy_l_tol<1.0",
                                    "The relative tolerance on the normalized residual in the "
                                    "linear solver of the energy equation.");
  params.addRangeCheckedParam<Real>("energy_l_abs_tol",
                                    1e-10,
                                    "0.0<energy_l_abs_tol",
                                    "The absolute tolerance on the normalized residual in the "
                                    "linear solver of the energy equation.");
  params.addRangeCheckedParam<unsigned int>(
      "energy_l_max_its",
      10000,
      "0<energy_l_max_its",
      "The maximum allowed iterations in the linear solver of the energy equation.");

  params.addParamNamesToGroup(
      "energy_equation_relaxation energy_petsc_options energy_petsc_options_iname "
      "energy_petsc_options_value energy_petsc_options_value energy_absolute_tolerance "
      "energy_l_tol energy_l_abs_tol energy_l_max_its",
      "Energy Equation");

  /*
   * Parameters to control the solution of the solid energy equation
   */

  params.addParam<MultiMooseEnum>("solid_energy_petsc_options",
                                  Moose::PetscSupport::getCommonPetscFlags(),
                                  "Singleton PETSc options for the solid energy equation");
  params.addParam<MultiMooseEnum>("solid_energy_petsc_options_iname",
                                  Moose::PetscSupport::getCommonPetscKeys(),
                                  "Names of PETSc name/value pairs for the solid energy equation");
  params.addParam<std::vector<std::string>>(
      "solid_energy_petsc_options_value",
      "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\" for the "
      "solid energy equation");

  params.addRangeCheckedParam<Real>(
      "solid_energy_absolute_tolerance",
      1e-5,
      "0.0<solid_energy_absolute_tolerance",
      "The absolute tolerance on the normalized residual of the solid energy equation.");

  params.addRangeCheckedParam<Real>("solid_energy_l_tol",
                                    1e-5,
                                    "0.0<=solid_energy_l_tol & solid_energy_l_tol<1.0",
                                    "The relative tolerance on the normalized residual in the "
                                    "linear solver of the solid energy equation.");

  params.addRangeCheckedParam<Real>("solid_energy_l_abs_tol",
                                    1e-10,
                                    "0.0<solid_energy_l_abs_tol",
                                    "The absolute tolerance on the normalized residual in the "
                                    "linear solver of the solid energy equation.");
  params.addRangeCheckedParam<unsigned int>(
      "solid_energy_l_max_its",
      10000,
      "0<solid_energy_l_max_its",
      "The maximum allowed iterations in the linear solver of the solid energy equation.");

  params.addParamNamesToGroup("solid_energy_petsc_options solid_energy_petsc_options_iname "
                              "solid_energy_petsc_options_value solid_energy_absolute_tolerance "
                              "solid_energy_l_tol solid_energy_l_abs_tol solid_energy_l_max_its",
                              "Solid Energy Equation");

  /*
   * Parameters to control the solution of each scalar advection system
   */
  params.addParam<std::vector<Real>>("passive_scalar_equation_relaxation",
                                     std::vector<Real>(),
                                     "The relaxation which should be used for the passive scalar "
                                     "equations. (=1 for no relaxation, "
                                     "diagonal dominance will still be enforced)");

  params.addParam<MultiMooseEnum>("passive_scalar_petsc_options",
                                  Moose::PetscSupport::getCommonPetscFlags(),
                                  "Singleton PETSc options for the passive scalar equation(s)");
  params.addParam<MultiMooseEnum>(
      "passive_scalar_petsc_options_iname",
      Moose::PetscSupport::getCommonPetscKeys(),
      "Names of PETSc name/value pairs for the passive scalar equation(s)");
  params.addParam<std::vector<std::string>>(
      "passive_scalar_petsc_options_value",
      "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\" for the "
      "passive scalar equation(s)");
  params.addParam<std::vector<Real>>(
      "passive_scalar_absolute_tolerance",
      std::vector<Real>(),
      "The absolute tolerance(s) on the normalized residual(s) of the passive scalar equation(s).");
  params.addRangeCheckedParam<Real>("passive_scalar_l_tol",
                                    1e-5,
                                    "0.0<=passive_scalar_l_tol & passive_scalar_l_tol<1.0",
                                    "The relative tolerance on the normalized residual in the "
                                    "linear solver of the passive scalar equation(s).");
  params.addRangeCheckedParam<Real>("passive_scalar_l_abs_tol",
                                    1e-10,
                                    "0.0<passive_scalar_l_abs_tol",
                                    "The absolute tolerance on the normalized residual in the "
                                    "linear solver of the passive scalar equation(s).");
  params.addParam<unsigned int>(
      "passive_scalar_l_max_its",
      10000,
      "The maximum allowed iterations in the linear solver of the turbulence equation.");

  params.addParamNamesToGroup(
      "passive_scalar_systems passive_scalar_equation_relaxation passive_scalar_petsc_options "
      "passive_scalar_petsc_options_iname "
      "passive_scalar_petsc_options_value passive_scalar_petsc_options_value "
      "passive_scalar_absolute_tolerance "
      "passive_scalar_l_tol passive_scalar_l_abs_tol passive_scalar_l_max_its",
      "passive_scalar Equation");

  /*
   * Parameters to control the solution of each turbulence system
   */
  params.addParam<std::vector<Real>>("turbulence_equation_relaxation",
                                     std::vector<Real>(),
                                     "The relaxation which should be used for the turbulence "
                                     "equations. (=1 for no relaxation, "
                                     "diagonal dominance will still be enforced)");

  params.addParam<MultiMooseEnum>("turbulence_petsc_options",
                                  Moose::PetscSupport::getCommonPetscFlags(),
                                  "Singleton PETSc options for the turbulence equation(s)");
  params.addParam<MultiMooseEnum>(
      "turbulence_petsc_options_iname",
      Moose::PetscSupport::getCommonPetscKeys(),
      "Names of PETSc name/value pairs for the turbulence equation(s)");
  params.addParam<std::vector<std::string>>(
      "turbulence_petsc_options_value",
      "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\" for the "
      "turbulence equation(s)");
  params.addParam<std::vector<Real>>(
      "turbulence_absolute_tolerance",
      std::vector<Real>(),
      "The absolute tolerance(s) on the normalized residual(s) of the turbulence equation(s).");
  params.addRangeCheckedParam<Real>("turbulence_l_tol",
                                    1e-5,
                                    "0.0<=turbulence_l_tol & turbulence_l_tol<1.0",
                                    "The relative tolerance on the normalized residual in the "
                                    "linear solver of the turbulence equation(s).");
  params.addRangeCheckedParam<Real>("turbulence_l_abs_tol",
                                    1e-10,
                                    "0.0<turbulence_l_abs_tol",
                                    "The absolute tolerance on the normalized residual in the "
                                    "linear solver of the turbulence equation(s).");
  params.addParam<unsigned int>(
      "turbulence_l_max_its",
      10000,
      "The maximum allowed iterations in the linear solver of the turbulence equation.");

  params.addParamNamesToGroup(
      "turbulence_systems turbulence_equation_relaxation turbulence_petsc_options "
      "turbulence_petsc_options_iname "
      "turbulence_petsc_options_value turbulence_petsc_options_value "
      "turbulence_absolute_tolerance "
      "turbulence_l_tol turbulence_l_abs_tol turbulence_l_max_its",
      "turbulence Equation");

  /*
   * SIMPLE iteration control
   */

  params.addRangeCheckedParam<unsigned int>(
      "num_iterations",
      1000,
      "0<num_iterations",
      "The number of momentum-pressure-(other fields) iterations needed.");

  params.addParam<bool>("continue_on_max_its",
                        false,
                        "If solve should continue if maximum number of iterations is hit.");

  return params;
}

SIMPLESolveBase::SIMPLESolveBase(Executioner & ex)
  : SolveObject(ex),
    UserObjectInterface(this),
    _momentum_system_names(getParam<std::vector<SolverSystemName>>("momentum_systems")),
    _momentum_l_abs_tol(getParam<Real>("momentum_l_abs_tol")),
    _momentum_equation_relaxation(getParam<Real>("momentum_equation_relaxation")),
    _pressure_system_name(getParam<SolverSystemName>("pressure_system")),
    _pressure_l_abs_tol(getParam<Real>("pressure_l_abs_tol")),
    _pressure_variable_relaxation(getParam<Real>("pressure_variable_relaxation")),
    _pin_pressure(getParam<bool>("pin_pressure")),
    _pressure_pin_value(getParam<Real>("pressure_pin_value")),
    _pressure_pin_dof(libMesh::invalid_uint),
    _has_energy_system(isParamValid("energy_system")),
    _energy_equation_relaxation(getParam<Real>("energy_equation_relaxation")),
    _energy_l_abs_tol(getParam<Real>("energy_l_abs_tol")),
    _has_solid_energy_system(_has_energy_system && isParamValid("solid_energy_system")),
    _solid_energy_l_abs_tol(getParam<Real>("solid_energy_l_abs_tol")),
    _passive_scalar_system_names(getParam<std::vector<SolverSystemName>>("passive_scalar_systems")),
    _has_passive_scalar_systems(!_passive_scalar_system_names.empty()),
    _passive_scalar_equation_relaxation(
        getParam<std::vector<Real>>("passive_scalar_equation_relaxation")),
    _passive_scalar_l_abs_tol(getParam<Real>("passive_scalar_l_abs_tol")),
    _turbulence_system_names(getParam<std::vector<SolverSystemName>>("turbulence_systems")),
    _has_turbulence_systems(!_turbulence_system_names.empty()),
    _turbulence_equation_relaxation(
        getParam<std::vector<Real>>("turbulence_equation_relaxation")),
    _turbulence_l_abs_tol(getParam<Real>("turbulence_l_abs_tol")),
    _momentum_absolute_tolerance(getParam<Real>("momentum_absolute_tolerance")),
    _pressure_absolute_tolerance(getParam<Real>("pressure_absolute_tolerance")),
    _energy_absolute_tolerance(getParam<Real>("energy_absolute_tolerance")),
    _solid_energy_absolute_tolerance(getParam<Real>("solid_energy_absolute_tolerance")),
    _passive_scalar_absolute_tolerance(
        getParam<std::vector<Real>>("passive_scalar_absolute_tolerance")),
    _turbulence_absolute_tolerance(
        getParam<std::vector<Real>>("turbulence_absolute_tolerance")),
    _num_iterations(getParam<unsigned int>("num_iterations")),
    _continue_on_max_its(getParam<bool>("continue_on_max_its")),
    _print_fields(getParam<bool>("print_fields"))
{
  if (_momentum_system_names.size() != _problem.mesh().dimension())
    paramError("momentum_systems",
               "The number of momentum components should be equal to the number of "
               "spatial dimensions on the mesh.");

  const auto & momentum_petsc_options = getParam<MultiMooseEnum>("momentum_petsc_options");
  const auto & momentum_petsc_pair_options = getParam<MooseEnumItem, std::string>(
      "momentum_petsc_options_iname", "momentum_petsc_options_value");
  Moose::PetscSupport::addPetscFlagsToPetscOptions(
      momentum_petsc_options, "-", *this, _momentum_petsc_options);
  Moose::PetscSupport::addPetscPairsToPetscOptions(momentum_petsc_pair_options,
                                                   _problem.mesh().dimension(),
                                                   "-",
                                                   *this,
                                                   _momentum_petsc_options);

  _momentum_linear_control.real_valued_data["rel_tol"] = getParam<Real>("momentum_l_tol");
  _momentum_linear_control.real_valued_data["abs_tol"] = getParam<Real>("momentum_l_abs_tol");
  _momentum_linear_control.int_valued_data["max_its"] =
      getParam<unsigned int>("momentum_l_max_its");

  const auto & pressure_petsc_options = getParam<MultiMooseEnum>("pressure_petsc_options");
  const auto & pressure_petsc_pair_options = getParam<MooseEnumItem, std::string>(
      "pressure_petsc_options_iname", "pressure_petsc_options_value");
  Moose::PetscSupport::addPetscFlagsToPetscOptions(
      pressure_petsc_options, "-", *this, _pressure_petsc_options);
  Moose::PetscSupport::addPetscPairsToPetscOptions(pressure_petsc_pair_options,
                                                   _problem.mesh().dimension(),
                                                   "-",
                                                   *this,
                                                   _pressure_petsc_options);

  _pressure_linear_control.real_valued_data["rel_tol"] = getParam<Real>("pressure_l_tol");
  _pressure_linear_control.real_valued_data["abs_tol"] = getParam<Real>("pressure_l_abs_tol");
  _pressure_linear_control.int_valued_data["max_its"] =
      getParam<unsigned int>("pressure_l_max_its");

  if (_has_energy_system)
  {
    const auto & energy_petsc_options = getParam<MultiMooseEnum>("energy_petsc_options");
    const auto & energy_petsc_pair_options = getParam<MooseEnumItem, std::string>(
        "energy_petsc_options_iname", "energy_petsc_options_value");
    Moose::PetscSupport::addPetscFlagsToPetscOptions(
        energy_petsc_options, "-", *this, _energy_petsc_options);
    Moose::PetscSupport::addPetscPairsToPetscOptions(
        energy_petsc_pair_options, _problem.mesh().dimension(), "-", *this, _energy_petsc_options);

    _energy_linear_control.real_valued_data["rel_tol"] = getParam<Real>("energy_l_tol");
    _energy_linear_control.real_valued_data["abs_tol"] = getParam<Real>("energy_l_abs_tol");
    _energy_linear_control.int_valued_data["max_its"] = getParam<unsigned int>("energy_l_max_its");
  }
  else
    checkDependentParameterError("energy_system",
                                 {"energy_petsc_options",
                                  "energy_petsc_options_iname",
                                  "energy_petsc_options_value",
                                  "energy_l_tol",
                                  "energy_l_abs_tol",
                                  "energy_l_max_its",
                                  "energy_absolute_tolerance",
                                  "energy_equation_relaxation"},
                                 false);

  if (_has_solid_energy_system)
  {
    const auto & solid_energy_petsc_options =
        getParam<MultiMooseEnum>("solid_energy_petsc_options");
    const auto & solid_energy_petsc_pair_options = getParam<MooseEnumItem, std::string>(
        "solid_energy_petsc_options_iname", "solid_energy_petsc_options_value");
    Moose::PetscSupport::addPetscFlagsToPetscOptions(
        solid_energy_petsc_options, "-", *this, _solid_energy_petsc_options);
    Moose::PetscSupport::addPetscPairsToPetscOptions(solid_energy_petsc_pair_options,
                                                     _problem.mesh().dimension(),
                                                     "-",
                                                     *this,
                                                     _solid_energy_petsc_options);

    _solid_energy_linear_control.real_valued_data["rel_tol"] = getParam<Real>("solid_energy_l_tol");
    _solid_energy_linear_control.real_valued_data["abs_tol"] =
        getParam<Real>("solid_energy_l_abs_tol");
    _solid_energy_linear_control.int_valued_data["max_its"] =
        getParam<unsigned int>("solid_energy_l_max_its");
  }
  else
    checkDependentParameterError("solid_energy_system",
                                 {"solid_energy_petsc_options",
                                  "solid_energy_petsc_options_iname",
                                  "solid_energy_petsc_options_value",
                                  "solid_energy_l_tol",
                                  "solid_energy_l_abs_tol",
                                  "solid_energy_l_max_its",
                                  "solid_energy_absolute_tolerance",
                                  "solid_energy_equation_relaxation"},
                                 false);

  // We check for input errors with regards to the passive scalar equations. At the same time, we
  // set up the corresponding system numbers
  if (_has_passive_scalar_systems)
  {
    if (_passive_scalar_system_names.size() != _passive_scalar_equation_relaxation.size())
      paramError("passive_scalar_equation_relaxation",
                 "The number of equation relaxation parameters does not match the number of "
                 "passive scalar equations!");
    if (_passive_scalar_system_names.size() != _passive_scalar_absolute_tolerance.size())
      paramError("passive_scalar_absolute_tolerance",
                 "The number of absolute tolerances does not match the number of "
                 "passive scalar equations!");
  }
  if (_has_passive_scalar_systems)
  {
    const auto & passive_scalar_petsc_options =
        getParam<MultiMooseEnum>("passive_scalar_petsc_options");
    const auto & passive_scalar_petsc_pair_options = getParam<MooseEnumItem, std::string>(
        "passive_scalar_petsc_options_iname", "passive_scalar_petsc_options_value");
    Moose::PetscSupport::addPetscFlagsToPetscOptions(
        passive_scalar_petsc_options, "-", *this, _passive_scalar_petsc_options);
    Moose::PetscSupport::addPetscPairsToPetscOptions(passive_scalar_petsc_pair_options,
                                                     _problem.mesh().dimension(),
                                                     "-",
                                                     *this,
                                                     _passive_scalar_petsc_options);

    _passive_scalar_linear_control.real_valued_data["rel_tol"] =
        getParam<Real>("passive_scalar_l_tol");
    _passive_scalar_linear_control.real_valued_data["abs_tol"] =
        getParam<Real>("passive_scalar_l_abs_tol");
    _passive_scalar_linear_control.int_valued_data["max_its"] =
        getParam<unsigned int>("passive_scalar_l_max_its");
  }
  else
    checkDependentParameterError("passive_scalar_systems",
                                 {"passive_scalar_petsc_options",
                                  "passive_scalar_petsc_options_iname",
                                  "passive_scalar_petsc_options_value",
                                  "passive_scalar_l_tol",
                                  "passive_scalar_l_abs_tol",
                                  "passive_scalar_l_max_its",
                                  "passive_scalar_equation_relaxation",
                                  "passive_scalar_absolute_tolerance"},
                                 false);

  // We check for input errors with regards to the surrogate turbulence equations. At the same time, we
  // set up the corresponding system numbers
  if (_has_turbulence_systems)
  {
    if (_turbulence_system_names.size() != _turbulence_equation_relaxation.size())
      paramError("turbulence_equation_relaxation",
                 "The number of equation relaxation parameters does not match the number of "
                 "passive scalar equations!");
    if (_turbulence_system_names.size() != _turbulence_absolute_tolerance.size())
      paramError("turbulence_absolute_tolerance",
                 "The number of absolute tolerances does not match the number of "
                 "passive scalar equations!");

    const auto & turbulence_petsc_options =
        getParam<MultiMooseEnum>("turbulence_petsc_options");
    const auto & turbulence_petsc_pair_options = getParam<MooseEnumItem, std::string>(
        "turbulence_petsc_options_iname", "turbulence_petsc_options_value");
    Moose::PetscSupport::addPetscFlagsToPetscOptions(
        turbulence_petsc_options, "-", *this, _turbulence_petsc_options);
    Moose::PetscSupport::addPetscPairsToPetscOptions(turbulence_petsc_pair_options,
                                                     _problem.mesh().dimension(),
                                                     "-",
                                                     *this,
                                                     _turbulence_petsc_options);

    _turbulence_linear_control.real_valued_data["rel_tol"] =
        getParam<Real>("turbulence_l_tol");
    _turbulence_linear_control.real_valued_data["abs_tol"] =
        getParam<Real>("turbulence_l_abs_tol");
    _turbulence_linear_control.int_valued_data["max_its"] =
        getParam<unsigned int>("turbulence_l_max_its");
  }
  else
    checkDependentParameterError("turbulence_systems",
                                 {"turbulence_petsc_options",
                                  "turbulence_petsc_options_iname",
                                  "turbulence_petsc_options_value",
                                  "turbulence_l_tol",
                                  "turbulence_l_abs_tol",
                                  "turbulence_l_max_its",
                                  "turbulence_equation_relaxation",
                                  "turbulence_absolute_tolerance"},
                                 false);
}

void
SIMPLESolveBase::setupPressurePin()
{
  if (_pin_pressure)
    _pressure_pin_dof = NS::FV::findPointDoFID(_problem.getVariable(0, "pressure"),
                                               _problem.mesh(),
                                               getParam<Point>("pressure_pin_point"));
}

void
SIMPLESolveBase::checkDependentParameterError(const std::string & main_parameter,
                                              const std::vector<std::string> & dependent_parameters,
                                              const bool should_be_defined)
{
  for (const auto & param : dependent_parameters)
    if (parameters().isParamSetByUser(param) == !should_be_defined)
      paramError(param,
                 "This parameter should " + std::string(should_be_defined ? "" : "not") +
                     " be given by the user with the corresponding " + main_parameter +
                     " setting!");
}
