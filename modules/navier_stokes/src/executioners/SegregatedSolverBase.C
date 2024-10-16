//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SegregatedSolverBase.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"
#include "KernelBase.h"
#include "INSFVMomentumPressure.h"
#include "libmesh/enum_point_locator_type.h"
#include "PetscVectorReader.h"

#include "libmesh/petsc_nonlinear_solver.h"
#include <petscerror.h>
#include <petscsys.h>
#include <petscksp.h>

InputParameters
SegregatedSolverBase::validParams()
{
  InputParameters params = Executioner::validParams();
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
      "passive_scalar_systems", {}, "The solver system(s) for the passive scalar equation(s).");
  params.addParam<std::vector<SolverSystemName>>(
      "turbulence_systems", {}, "The solver system(s) for the turbulence equation(s).");

  /*
   * Relaxation parameters for the different system
   */
  params.addRangeCheckedParam<Real>(
      "pressure_variable_relaxation",
      1.0,
      "0.0<pressure_variable_relaxation<=1.0",
      "The relaxation which should be used for the pressure variable (=1 for no relaxation).");
  params.addRangeCheckedParam<Real>(
      "momentum_equation_relaxation",
      1.0,
      "0.0<momentum_equation_relaxation<=1.0",
      "The relaxation which should be used for the momentum equation. (=1 for no relaxation, "
      "diagonal dominance will still be enforced)");
  params.addRangeCheckedParam<Real>(
      "energy_equation_relaxation",
      1.0,
      "0.0<energy_equation_relaxation<=1.0",
      "The relaxation which should be used for the energy equation. (=1 for no relaxation, "
      "diagonal dominance will still be enforced)");
  params.addParam<std::vector<Real>>("passive_scalar_equation_relaxation",
                                     std::vector<Real>(),
                                     "The relaxation which should be used for the passive scalar "
                                     "equations. (=1 for no relaxation, "
                                     "diagonal dominance will still be enforced)");
  params.addParam<std::vector<Real>>(
      "turbulence_equation_relaxation",
      std::vector<Real>(),
      "The relaxation which should be used for the turbulence equations "
      "equations. (=1 for no relaxation, "
      "diagonal dominance will still be enforced)");

  params.addParam<std::vector<Real>>(
      "turbulence_field_min_limit",
      std::vector<Real>(),
      "The lower limit imposed on turbulent quantities. The recommended value for robustness "
      "is 1e-8.");

  params.addParamNamesToGroup("pressure_variable_relaxation momentum_equation_relaxation "
                              "energy_equation_relaxation passive_scalar_equation_relaxation "
                              "turbulence_equation_relaxation",
                              "Relaxation");

  /*
   * Petsc options for every equations in the system
   */
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

  params.addParam<MultiMooseEnum>("turbulence_petsc_options",
                                  Moose::PetscSupport::getCommonPetscFlags(),
                                  "Singleton PETSc options for the turbulence equation(s)");
  params.addParam<MultiMooseEnum>("turbulence_petsc_options_iname",
                                  Moose::PetscSupport::getCommonPetscKeys(),
                                  "Names of PETSc name/value pairs for the turbulence equation(s)");
  params.addParam<std::vector<std::string>>(
      "turbulence_petsc_options_value",
      "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\" for the "
      "turbulence equation");

  params.addParamNamesToGroup(
      "momentum_petsc_options momentum_petsc_options_iname momentum_petsc_options_value "
      "pressure_petsc_options pressure_petsc_options_iname pressure_petsc_options_value "
      "energy_petsc_options energy_petsc_options_iname energy_petsc_options_value "
      "solid_energy_petsc_options solid_energy_petsc_options_iname "
      "solid_energy_petsc_options_value passive_scalar_petsc_options "
      "passive_scalar_petsc_options_iname passive_scalar_petsc_options_value "
      "turbulence_petsc_options turbulence_petsc_options_iname turbulence_petsc_options_value",
      "PETSc Control");

  /*
   * Iteration tolerances for the different equations
   */
  params.addRangeCheckedParam<Real>(
      "momentum_absolute_tolerance",
      1e-5,
      "0.0<momentum_absolute_tolerance",
      "The absolute tolerance on the normalized residual of the momentum equation.");
  params.addRangeCheckedParam<Real>(
      "pressure_absolute_tolerance",
      1e-5,
      "0.0<pressure_absolute_tolerance",
      "The absolute tolerance on the normalized residual of the pressure equation.");
  params.addRangeCheckedParam<Real>(
      "energy_absolute_tolerance",
      1e-5,
      "0.0<energy_absolute_tolerance",
      "The absolute tolerance on the normalized residual of the energy equation.");
  params.addRangeCheckedParam<Real>(
      "solid_energy_absolute_tolerance",
      1e-5,
      "0.0<solid_energy_absolute_tolerance",
      "The absolute tolerance on the normalized residual of the solid energy equation.");
  params.addParam<std::vector<Real>>(
      "passive_scalar_absolute_tolerance",
      std::vector<Real>(),
      "The absolute tolerance(s) on the normalized residual(s) of the passive scalar equation(s).");
  params.addParam<std::vector<Real>>(
      "turbulence_absolute_tolerance",
      std::vector<Real>(),
      "The absolute tolerance(s) on the normalized residual(s) of the turbulence equation(s).");
  params.addRangeCheckedParam<unsigned int>(
      "num_iterations",
      1000,
      "0<num_iterations",
      "The number of momentum-pressure-(other fields) iterations needed.");

  params.addParamNamesToGroup(
      "momentum_absolute_tolerance pressure_absolute_tolerance energy_absolute_tolerance "
      "solid_energy_absolute_tolerance passive_scalar_absolute_tolerance "
      "turbulence_absolute_tolerance num_iterations",
      "Iteration Control");
  /*
   * Linear iteration tolerances for the different equations
   */
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
      "The maximum allowed iterations in the linear solver of the turbulence equation(s).");

  params.addParamNamesToGroup(
      "momentum_l_tol momentum_l_abs_tol momentum_l_max_its pressure_l_tol pressure_l_abs_tol "
      "pressure_l_max_its solid_energy_l_tol solid_energy_l_abs_tol solid_energy_l_max_its "
      "energy_l_tol energy_l_abs_tol energy_l_max_its passive_scalar_l_tol "
      "passive_scalar_l_abs_tol passive_scalar_l_max_its turbulence_l_tol "
      "turbulence_l_abs_tol turbulence_l_max_its",
      "Linear Iteration Control");

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
   * We suppress parameters which are not supported yet
   */
  params.suppressParameter<MooseEnum>("fixed_point_algorithm");
  params.suppressParameter<unsigned int>("fixed_point_min_its");
  params.suppressParameter<unsigned int>("fixed_point_max_its");
  params.suppressParameter<bool>("accept_on_max_fixed_point_iteration");
  params.suppressParameter<bool>("disable_fixed_point_residual_norm_check");
  params.suppressParameter<Real>("fixed_point_rel_tol");
  params.suppressParameter<Real>("fixed_point_abs_tol");
  params.suppressParameter<unsigned int>("fixed_point_min_its");
  params.suppressParameter<bool>("fixed_point_force_norms");
  params.suppressParameter<PostprocessorName>("custom_pp");
  params.suppressParameter<Real>("custom_rel_tol");
  params.suppressParameter<Real>("custom_abs_tol");
  params.suppressParameter<bool>("direct_pp_value");
  params.suppressParameter<Real>("relaxation_factor");
  params.suppressParameter<std::vector<std::string>>("transformed_variables");
  params.suppressParameter<std::vector<PostprocessorName>>("transformed_postprocessors");
  params.suppressParameter<std::vector<std::string>>("relaxed_variables");
  params.suppressParameter<bool>("auto_advance");
  params.suppressParameter<unsigned int>("max_xfem_update");
  params.suppressParameter<bool>("update_xfem_at_timestep_begin");

  return params;
}

SegregatedSolverBase::SegregatedSolverBase(const InputParameters & parameters)
  : Executioner(parameters),
    _problem(_fe_problem),
    _time_step(_problem.timeStep()),
    _time(_problem.time()),
    _has_energy_system(isParamValid("energy_system")),
    _has_solid_energy_system(_has_energy_system && isParamValid("solid_energy_system")),
    _has_passive_scalar_systems(isParamValid("passive_scalar_systems")),
    _has_turbulence_systems(isParamValid("turbulence_systems")),
    _momentum_system_names(getParam<std::vector<SolverSystemName>>("momentum_systems")),
    _passive_scalar_system_names(getParam<std::vector<SolverSystemName>>("passive_scalar_systems")),
    _turbulence_system_names(getParam<std::vector<SolverSystemName>>("turbulence_systems")),
    _momentum_equation_relaxation(getParam<Real>("momentum_equation_relaxation")),
    _pressure_variable_relaxation(getParam<Real>("pressure_variable_relaxation")),
    _energy_equation_relaxation(getParam<Real>("energy_equation_relaxation")),
    _passive_scalar_equation_relaxation(
        getParam<std::vector<Real>>("passive_scalar_equation_relaxation")),
    _turbulence_equation_relaxation(getParam<std::vector<Real>>("turbulence_equation_relaxation")),
    _turbulence_field_min_limit(getParam<std::vector<Real>>("turbulence_field_min_limit")),
    _momentum_absolute_tolerance(getParam<Real>("momentum_absolute_tolerance")),
    _pressure_absolute_tolerance(getParam<Real>("pressure_absolute_tolerance")),
    _energy_absolute_tolerance(getParam<Real>("energy_absolute_tolerance")),
    _solid_energy_absolute_tolerance(getParam<Real>("solid_energy_absolute_tolerance")),
    _passive_scalar_absolute_tolerance(
        getParam<std::vector<Real>>("passive_scalar_absolute_tolerance")),
    _turbulence_absolute_tolerance(getParam<std::vector<Real>>("turbulence_absolute_tolerance")),
    _num_iterations(getParam<unsigned int>("num_iterations")),
    _print_fields(getParam<bool>("print_fields")),
    _momentum_l_abs_tol(getParam<Real>("momentum_l_abs_tol")),
    _pressure_l_abs_tol(getParam<Real>("pressure_l_abs_tol")),
    _energy_l_abs_tol(getParam<Real>("energy_l_abs_tol")),
    _solid_energy_l_abs_tol(getParam<Real>("solid_energy_l_abs_tol")),
    _passive_scalar_l_abs_tol(getParam<Real>("passive_scalar_l_abs_tol")),
    _turbulence_l_abs_tol(getParam<Real>("turbulence_l_abs_tol")),
    _pin_pressure(getParam<bool>("pin_pressure")),
    _pressure_pin_value(getParam<Real>("pressure_pin_value"))
{
  if (_momentum_system_names.size() != _problem.mesh().dimension())
    paramError("momentum_systems",
               "The number of momentum components should be equal to the number of "
               "spatial dimensions on the mesh.");

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

  // We check for input errors with regards to the turbulence equations. At the same time, we
  // set up the corresponding system numbers
  if (_has_turbulence_systems)
  {
    if (_turbulence_system_names.size() != _turbulence_equation_relaxation.size())
      paramError("turbulence_equation_relaxation",
                 "The number of equation relaxation parameters does not match the number of "
                 "turbulence scalar equations!");
    if (_turbulence_system_names.size() != _turbulence_absolute_tolerance.size())
      paramError("turbulence_absolute_tolerance",
                 "The number of absolute tolerances does not match the number of "
                 "turbulence equations!");
    if (_turbulence_field_min_limit.empty())
      // If no minimum bounds are given, initialize to default value 1e-8
      _turbulence_field_min_limit.resize(_turbulence_system_names.size(), 1e-8);
    else if (_turbulence_system_names.size() != _turbulence_field_min_limit.size())
      paramError("turbulence_field_min_limit",
                 "The number of lower bounds for turbulent quantities does not match the "
                 "number of turbulence equations!");
  }

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

  if (isParamValid("solid_energy_system") && !_has_energy_system)
    paramError(
        "solid_energy_system",
        "We cannot solve a solid energy system without solving for the fluid energy as well!");

  if (_has_energy_system)
  {
    const auto & energy_petsc_options = getParam<MultiMooseEnum>("energy_petsc_options");
    const auto & energy_petsc_pair_options = getParam<MooseEnumItem, std::string>(
        "energy_petsc_options_iname", "energy_petsc_options_value");
    Moose::PetscSupport::processPetscFlags(energy_petsc_options, _energy_petsc_options);
    Moose::PetscSupport::processPetscPairs(
        energy_petsc_pair_options, _problem.mesh().dimension(), _energy_petsc_options);

    _energy_linear_control.real_valued_data["rel_tol"] = getParam<Real>("energy_l_tol");
    _energy_linear_control.real_valued_data["abs_tol"] = getParam<Real>("energy_l_abs_tol");
    _energy_linear_control.int_valued_data["max_its"] = getParam<unsigned int>("energy_l_max_its");

    // We only allow the solve for a solid energy system if we already solve for the fluid energy
    if (_has_solid_energy_system)
    {
      const auto & solid_energy_petsc_options =
          getParam<MultiMooseEnum>("solid_energy_petsc_options");
      const auto & solid_energy_petsc_pair_options = getParam<MooseEnumItem, std::string>(
          "solid_energy_petsc_options_iname", "solid_energy_petsc_options_value");
      Moose::PetscSupport::processPetscFlags(solid_energy_petsc_options,
                                             _solid_energy_petsc_options);
      Moose::PetscSupport::processPetscPairs(solid_energy_petsc_pair_options,
                                             _problem.mesh().dimension(),
                                             _solid_energy_petsc_options);

      _solid_energy_linear_control.real_valued_data["rel_tol"] =
          getParam<Real>("solid_energy_l_tol");
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
                                    "solid_energy_absolute_tolerance"},
                                   false);
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

  if (_has_passive_scalar_systems)
  {
    const auto & passive_scalar_petsc_options =
        getParam<MultiMooseEnum>("passive_scalar_petsc_options");
    const auto & passive_scalar_petsc_pair_options = getParam<MooseEnumItem, std::string>(
        "passive_scalar_petsc_options_iname", "passive_scalar_petsc_options_value");
    Moose::PetscSupport::processPetscFlags(passive_scalar_petsc_options,
                                           _passive_scalar_petsc_options);
    Moose::PetscSupport::processPetscPairs(passive_scalar_petsc_pair_options,
                                           _problem.mesh().dimension(),
                                           _passive_scalar_petsc_options);

    _passive_scalar_linear_control.real_valued_data["rel_tol"] =
        getParam<Real>("passive_scalar_l_tol");
    _passive_scalar_linear_control.real_valued_data["abs_tol"] =
        getParam<Real>("passive_scalar_l_abs_tol");
    _passive_scalar_linear_control.int_valued_data["max_its"] =
        getParam<unsigned int>("passive_scalar_l_max_its");
  }
  else
    checkDependentParameterError("passive_scalar_system",
                                 {"passive_scalar_petsc_options",
                                  "passive_scalar_petsc_options_iname",
                                  "passive_scalar_petsc_options_value",
                                  "passive_scalar_l_tol",
                                  "passive_scalar_l_abs_tol",
                                  "passive_scalar_l_max_its",
                                  "passive_scalar_equation_relaxation",
                                  "passive_scalar_absolute_tolerance"},
                                 false);

  if (_has_turbulence_systems)
  {
    const auto & turbulence_petsc_options = getParam<MultiMooseEnum>("turbulence_petsc_options");
    const auto & turbulence_petsc_pair_options = getParam<MooseEnumItem, std::string>(
        "turbulence_petsc_options_iname", "turbulence_petsc_options_value");
    Moose::PetscSupport::processPetscFlags(turbulence_petsc_options, _turbulence_petsc_options);
    Moose::PetscSupport::processPetscPairs(
        turbulence_petsc_pair_options, _problem.mesh().dimension(), _turbulence_petsc_options);

    _turbulence_linear_control.real_valued_data["rel_tol"] = getParam<Real>("turbulence_l_tol");
    _turbulence_linear_control.real_valued_data["abs_tol"] = getParam<Real>("turbulence_l_abs_tol");
    _turbulence_linear_control.int_valued_data["max_its"] =
        getParam<unsigned int>("turbulence_l_max_its");
  }
  else
    checkDependentParameterError("turbulence_system",
                                 {"turbulence_petsc_options",
                                  "turbulence_petsc_options_iname",
                                  "turbulence_petsc_options_value",
                                  "turbulence_l_tol",
                                  "turbulence_l_abs_tol",
                                  "turbulence_l_max_its",
                                  "turbulence_equation_relaxation",
                                  "turbulence_absolute_tolerance"},
                                 false);

  _time = 0;
  // This class of segregated solvers for the Navier Stokes equations does not require preserving
  // entries in a Jacobian
  _problem.setPreserveMatrixSparsityPattern(false);
}

void
SegregatedSolverBase::checkDependentParameterError(
    const std::string & main_parameter,
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

bool
SegregatedSolverBase::hasMultiAppError(const ExecFlagEnum & flags)
{
  bool has_error = false;
  for (const auto & flag : flags)
    if (_problem.hasMultiApps(flag))
    {
      _console << "\nCannot use SegregatedSolverBase solves with MultiApps set to execute on "
               << flag.name() << "!\nExiting...\n"
               << std::endl;
      has_error = true;
    }

  return has_error;
}

bool
SegregatedSolverBase::hasTransferError(const ExecFlagEnum & flags)
{
  for (const auto & flag : flags)
    if (_problem.getTransfers(flag, MultiAppTransfer::TO_MULTIAPP).size() ||
        _problem.getTransfers(flag, MultiAppTransfer::FROM_MULTIAPP).size() ||
        _problem.getTransfers(flag, MultiAppTransfer::BETWEEN_MULTIAPP).size())
    {
      _console
          << "\nCannot use SegregatedSolverBase solves with MultiAppTransfers set to execute on "
          << flag.name() << "!\nExiting...\n"
          << std::endl;
      return true;
    }

  return false;
}

void
SegregatedSolverBase::init()
{
  _problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _problem.initialSetup();

  if (_pin_pressure)
    _pressure_pin_dof = findDoFID("pressure", getParam<Point>("pressure_pin_point"));
}

void
SegregatedSolverBase::relaxMatrix(SparseMatrix<Number> & matrix,
                                  const Real relaxation_parameter,
                                  NumericVector<Number> & diff_diagonal)
{
  PetscMatrix<Number> * mat = dynamic_cast<PetscMatrix<Number> *>(&matrix);
  mooseAssert(mat, "This should be a PetscMatrix!");
  PetscVector<Number> * diff_diag = dynamic_cast<PetscVector<Number> *>(&diff_diagonal);
  mooseAssert(diff_diag, "This should be a PetscVector!");

  // Zero the diagonal difference vector
  *diff_diag = 0;

  // Get the diagonal of the matrix
  mat->get_diagonal(*diff_diag);

  // Create a copy of the diagonal for later use and cast it
  auto original_diagonal = diff_diag->clone();

  // We cache the inverse of the relaxation parameter because doing divisions might
  // be more expensive for every row
  const Real inverse_relaxation = 1 / relaxation_parameter;

  // Now we loop over the matrix row by row and sum the absolute values of the
  // offdiagonal values, If these sums are larger than the diagonal entry,
  // we switch the diagonal value with the sum. At the same time we increase the
  // diagonal by dividing it with the relaxation parameter. So the new diagonal will be:
  // D* = 1/lambda*max(|D|,sum(|Offdiag|))
  // For more information see
  //
  // Juretic, Franjo. Error analysis in finite volume CFD. Diss.
  // Imperial College London (University of London), 2005.
  //
  // The trickery comes with storing everything in the diff-diagonal vector
  // to avoid the allocation and manipulation of a third vector
  const unsigned int local_size = matrix.row_stop() - matrix.row_start();
  std::vector<dof_id_type> indices(local_size);
  std::iota(indices.begin(), indices.end(), matrix.row_start());
  std::vector<Real> new_diagonal(local_size, 0.0);

  {
    PetscVectorReader diff_diga_reader(*diff_diag);
    for (const auto row_i : make_range(local_size))
    {
      const unsigned int global_index = matrix.row_start() + row_i;
      std::vector<numeric_index_type> indices;
      std::vector<Real> values;
      mat->get_row(global_index, indices, values);
      const Real abs_sum = std::accumulate(
          values.cbegin(), values.cend(), 0.0, [](Real a, Real b) { return a + std::abs(b); });
      const Real abs_diagonal = std::abs(diff_diga_reader(global_index));
      new_diagonal[row_i] = inverse_relaxation * std::max(abs_sum - abs_diagonal, abs_diagonal);
    }
  }
  diff_diag->insert(new_diagonal, indices);

  // Time to modify the diagonal of the matrix. TODO: add this function to libmesh
  LIBMESH_CHKERR(MatDiagonalSet(mat->mat(), diff_diag->vec(), INSERT_VALUES));
  mat->close();
  diff_diag->close();

  // Finally, we can create (D*-D) vector which is used for the relaxation of the
  // right hand side later
  diff_diag->add(-1.0, *original_diagonal);
}

void
SegregatedSolverBase::relaxRightHandSide(NumericVector<Number> & rhs,
                                         const NumericVector<Number> & solution,
                                         const NumericVector<Number> & diff_diagonal)
{

  // We need a working vector here to make sure we don't modify the
  // (D*-D) vector
  auto working_vector = diff_diagonal.clone();
  working_vector->pointwise_mult(solution, *working_vector);

  // The correction to the right hand side is just
  // (D*-D)*old_solution
  // For more information see
  //
  // Juretic, Franjo. Error analysis in finite volume CFD. Diss.
  // Imperial College London (University of London), 2005.
  rhs.add(*working_vector);
  rhs.close();
}

Real
SegregatedSolverBase::computeNormalizationFactor(const NumericVector<Number> & solution,
                                                 const SparseMatrix<Number> & mat,
                                                 const NumericVector<Number> & rhs)
{
  // This function is based on the description provided here:
  // @article{greenshields2022notes,
  // title={Notes on computational fluid dynamics: General principles},
  // author={Greenshields, Christopher J and Weller, Henry G},
  // journal={(No Title)},
  // year={2022}
  // }
  // so basically we normalize the residual with the following number:
  // sum(|Ax-Ax_avg|+|b-Ax_avg|)
  // where A is the system matrix, b is the system right hand side while x and x_avg are
  // the solution and average solution vectors

  // We create a vector for Ax_avg and Ax
  auto A_times_average_solution = solution.zero_clone();
  auto A_times_solution = solution.zero_clone();

  // Beware, trickery here! To avoid allocating unused vectors, we
  // first compute Ax_avg using the storage used for Ax, then we
  // overwrite Ax with the right value
  *A_times_solution = solution.sum() / solution.size();
  mat.vector_mult(*A_times_average_solution, *A_times_solution);
  mat.vector_mult(*A_times_solution, solution);

  // We create Ax-Ax_avg
  A_times_solution->add(-1.0, *A_times_average_solution);
  // We create Ax_avg - b (ordering shouldn't matter we will take absolute value soon)
  A_times_average_solution->add(-1.0, rhs);
  A_times_solution->abs();
  A_times_average_solution->abs();

  // Create |Ax-Ax_avg|+|b-Ax_avg|
  A_times_average_solution->add(*A_times_solution);

  // Since use the l2 norm of the solution vectors in the linear solver, we will
  // make this consistent and use the l2 norm of the vector. We add a small number to
  // avoid normalizing with 0.
  // TODO: Would be nice to see if we can do l1 norms in the linear solve.
  return (A_times_average_solution->l2_norm() + libMesh::TOLERANCE * libMesh::TOLERANCE);
}

void
SegregatedSolverBase::constrainSystem(SparseMatrix<Number> & mx,
                                      NumericVector<Number> & rhs,
                                      const Real desired_value,
                                      const dof_id_type dof_id)
{
  // Modify the given matrix and right hand side. We use the matrix diagonal
  // to enforce the constraint instead of 1, to make sure we don't mess up the matrix conditioning
  // too much.
  if (dof_id >= mx.row_start() && dof_id < mx.row_stop())
  {
    Real diag = mx(dof_id, dof_id);
    rhs.add(dof_id, desired_value * diag);
    mx.add(dof_id, dof_id, diag);
  }
}

dof_id_type
SegregatedSolverBase::findDoFID(const VariableName & var_name, const Point & point)
{
  // Find the element containing the point
  auto point_locator = PointLocatorBase::build(TREE_LOCAL_ELEMENTS, _fe_problem.mesh());
  point_locator->enable_out_of_mesh_mode();

  const auto & variable = _fe_problem.getVariable(0, var_name);
  unsigned int var_num = variable.sys().system().variable_number(var_name);

  // We only check in the restricted blocks, if needed
  const bool block_restricted =
      variable.blockIDs().find(Moose::ANY_BLOCK_ID) == variable.blockIDs().end();
  const Elem * elem =
      block_restricted ? (*point_locator)(point, &variable.blockIDs()) : (*point_locator)(point);

  // We communicate the results and if there is conflict between processes,
  // the minimum cell ID is chosen
  const dof_id_type elem_id = elem ? elem->id() : DofObject::invalid_id;
  dof_id_type min_elem_id = elem_id;
  comm().min(min_elem_id);

  if (min_elem_id == DofObject::invalid_id)
    mooseError("Variable ",
               var_name,
               " is not defined at ",
               Moose::stringify(point),
               "! Try alleviating block restrictions or using another point!");

  return min_elem_id == elem_id ? elem->dof_number(variable.sys().number(), var_num, 0)
                                : DofObject::invalid_id;
}

void
SegregatedSolverBase::relaxSolutionUpdate(NumericVector<Number> & vec_new,
                                          const NumericVector<Number> & vec_old,
                                          const Real relaxation_factor)
{
  // The relaxation is just u = lambda * u* + (1-lambda) u_old
  vec_new.scale(relaxation_factor);
  vec_new.add(1 - relaxation_factor, vec_old);
  vec_new.close();

  if (_print_fields)
  {
    _console << "Pressure solution" << std::endl;
    vec_new.print();
    _console << "Pressure solution old" << std::endl;
    vec_old.print();
  }
}

void
SegregatedSolverBase::limitSolutionUpdate(NumericVector<Number> & solution,
                                          const Real min_limit,
                                          const Real max_limit)
{
  PetscVector<Number> & solution_vector = dynamic_cast<PetscVector<Number> &>(solution);
  auto value = solution_vector.get_array();

  for (auto i : make_range(solution_vector.local_size()))
    value[i] = std::min(std::max(min_limit, value[i]), max_limit);

  solution_vector.restore_array();
}

bool
SegregatedSolverBase::converged(
    const std::vector<std::pair<unsigned int, Real>> & its_and_residuals,
    const std::vector<Real> & abs_tolerances)
{
  mooseAssert(its_and_residuals.size() == abs_tolerances.size(),
              "The number of residuals should (now " + std::to_string(its_and_residuals.size()) +
                  ") be the same as the number of tolerances (" +
                  std::to_string(abs_tolerances.size()) + ")!");

  bool converged = true;
  for (const auto system_i : index_range(its_and_residuals))
  {
    converged = converged && (its_and_residuals[system_i].second < abs_tolerances[system_i]);
    if (!converged)
      return converged;
  }
  return converged;
}
