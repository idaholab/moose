//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SIMPLE.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"
#include "KernelBase.h"
#include "INSFVMomentumPressure.h"
#include "libmesh/enum_point_locator_type.h"

#include "libmesh/petsc_nonlinear_solver.h"
#include <petscerror.h>
#include <petscsys.h>
#include <petscksp.h>

registerMooseObject("NavierStokesApp", SIMPLE);

InputParameters
SIMPLE::validParams()
{
  InputParameters params = Executioner::validParams();
  params.addRequiredParam<UserObjectName>("rhie_chow_user_object", "The rhie-chow user-object");

  params.addClassDescription("Solves the Navier-Stokes equations using the SIMPLE algorithm.");

  /*
   * The names of the different systems in the segregated solver
   */
  params.addRequiredParam<std::vector<NonlinearSystemName>>(
      "momentum_systems", "The nonlinear system(s) for the momentum equation(s).");
  params.addRequiredParam<NonlinearSystemName>("pressure_system",
                                               "The nonlinear system for the pressure equation.");
  params.addParam<NonlinearSystemName>("energy_system",
                                       "The nonlinear system for the energy equation.");
  params.addParam<NonlinearSystemName>("solid_energy_system",
                                       "The nonlinear system for the solid energy equation.");
  params.addParam<std::vector<NonlinearSystemName>>(
      "passive_scalar_systems", "The nonlinear system(s) for the passive scalar equation(s).");
  params.addParam<TagName>("pressure_gradient_tag",
                           "pressure_momentum_kernels",
                           "The name of the tags associated with the kernels in the momentum "
                           "equations which are not related to the pressure gradient.");

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

  params.addParamNamesToGroup("pressure_variable_relaxation momentum_equation_relaxation "
                              "energy_equation_relaxation passive_scalar_equation_relaxation",
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
                                  "Singleton PETSc options for the energy equation");
  params.addParam<MultiMooseEnum>("passive_scalar_petsc_options_iname",
                                  Moose::PetscSupport::getCommonPetscKeys(),
                                  "Names of PETSc name/value pairs for the energy equation");
  params.addParam<std::vector<std::string>>(
      "passive_scalar_petsc_options_value",
      "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\" for the "
      "energy equation");

  params.addParamNamesToGroup(
      "momentum_petsc_options momentum_petsc_options_iname momentum_petsc_options_value "
      "pressure_petsc_options pressure_petsc_options_iname pressure_petsc_options_value "
      "energy_petsc_options energy_petsc_options_iname energy_petsc_options_value "
      "solid_energy_petsc_options solid_energy_petsc_options_iname "
      "solid_energy_petsc_options_value passive_scalar_petsc_options "
      "passive_scalar_petsc_options_iname passive_scalar_petsc_options_value",
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
  params.addRangeCheckedParam<unsigned int>("num_iterations",
                                            1000,
                                            "0<num_iterations",
                                            "The number of momentum-pressure iterations needed.");

  params.addRangeCheckedParam<unsigned int>(
      "num_iterations",
      1000,
      "0<num_iterations",
      "The number of momentum-pressure-(other fields) iterations needed.");

  params.addParamNamesToGroup(
      "momentum_absolute_tolerance pressure_absolute_tolerance energy_absolute_tolerance "
      "solid_energy_absolute_tolerance passive_scalar_absolute_tolerance num_iterations",
      "Nonlinear Iteration");
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
      "The maximum allowed iterations in the linear solver of the passive scalar equation.");

  params.addParamNamesToGroup(
      "momentum_l_tol momentum_l_abs_tol momentum_l_max_its pressure_l_tol pressure_l_abs_tol "
      "pressure_l_max_its solid_energy_l_tol solid_energy_l_abs_tol solid_energy_l_max_its "
      "energy_l_tol energy_l_abs_tol energy_l_max_its passive_scalar_l_tol "
      "passive_scalar_l_abs_tol passive_scalar_l_max_its",
      "Linear Iteration");

  /*
   * Pressure pin parameters for enclosed flows
   */

  params.addParam<bool>(
      "pin_pressure", false, "If the pressure field needs to be pinned at a point.");
  params.addParam<Real>(
      "pressure_pin_value", 0, "The value which needs to be enforced for the pressure.");
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

SIMPLE::SIMPLE(const InputParameters & parameters)
  : Executioner(parameters),
    _problem(_fe_problem),
    _time_step(_problem.timeStep()),
    _time(_problem.time()),
    _has_energy_system(isParamValid("energy_system")),
    _has_solid_energy_system(_has_energy_system && isParamValid("solid_energy_system")),
    _has_passive_scalar_systems(isParamValid("passive_scalar_systems")),
    _momentum_system_names(getParam<std::vector<NonlinearSystemName>>("momentum_systems")),
    _pressure_sys_number(_problem.nlSysNum(getParam<NonlinearSystemName>("pressure_system"))),
    _energy_sys_number(_has_energy_system
                           ? _problem.nlSysNum(getParam<NonlinearSystemName>("energy_system"))
                           : libMesh::invalid_uint),
    _solid_energy_sys_number(
        _has_solid_energy_system
            ? _problem.nlSysNum(getParam<NonlinearSystemName>("solid_energy_system"))
            : libMesh::invalid_uint),
    _pressure_system(_problem.getNonlinearSystemBase(_pressure_sys_number)),
    _energy_system(_has_energy_system ? &_problem.getNonlinearSystemBase(_energy_sys_number)
                                      : nullptr),
    _solid_energy_system(_has_solid_energy_system
                             ? &_problem.getNonlinearSystemBase(_solid_energy_sys_number)
                             : nullptr),
    _pressure_tag_name(getParam<TagName>("pressure_gradient_tag")),
    _pressure_tag_id(_problem.addVectorTag(_pressure_tag_name)),
    _momentum_equation_relaxation(getParam<Real>("momentum_equation_relaxation")),
    _pressure_variable_relaxation(getParam<Real>("pressure_variable_relaxation")),
    _energy_equation_relaxation(getParam<Real>("energy_equation_relaxation")),
    _passive_scalar_equation_relaxation(
        getParam<std::vector<Real>>("passive_scalar_equation_relaxation")),
    _momentum_absolute_tolerance(getParam<Real>("momentum_absolute_tolerance")),
    _pressure_absolute_tolerance(getParam<Real>("pressure_absolute_tolerance")),
    _energy_absolute_tolerance(getParam<Real>("energy_absolute_tolerance")),
    _solid_energy_absolute_tolerance(getParam<Real>("solid_energy_absolute_tolerance")),
    _passive_scalar_absolute_tolerance(
        getParam<std::vector<Real>>("passive_scalar_absolute_tolerance")),
    _num_iterations(getParam<unsigned int>("num_iterations")),
    _print_fields(getParam<bool>("print_fields")),
    _momentum_l_abs_tol(getParam<Real>("momentum_l_abs_tol")),
    _pressure_l_abs_tol(getParam<Real>("pressure_l_abs_tol")),
    _energy_l_abs_tol(getParam<Real>("energy_l_abs_tol")),
    _solid_energy_l_abs_tol(getParam<Real>("solid_energy_l_abs_tol")),
    _passive_scalar_l_abs_tol(getParam<Real>("passive_scalar_l_abs_tol")),
    _pin_pressure(getParam<bool>("pin_pressure")),
    _pressure_pin_value(getParam<Real>("pressure_pin_value"))

{
  if (_momentum_system_names.size() != _problem.mesh().dimension())
    paramError("momentum_systems",
               "The number of momentum components should be equal to the number of "
               "spatial dimensions on the mesh.");

  // We fetch the system numbers for the momentum components plus add vectors
  // for removing the contribution from the pressure gradient terms.
  for (auto system_i : index_range(_momentum_system_names))
  {
    _momentum_system_numbers.push_back(_problem.nlSysNum(_momentum_system_names[system_i]));
    _momentum_systems.push_back(
        &_problem.getNonlinearSystemBase(_momentum_system_numbers[system_i]));
    _momentum_systems[system_i]->addVector(_pressure_tag_id, false, ParallelType::PARALLEL);
  }

  // We check for input errors with regards to the passive scalar equations. At the same time, we
  // set up the corresponding system numbers
  if (_has_passive_scalar_systems)
  {
    const auto & passive_scalar_system_names =
        getParam<std::vector<NonlinearSystemName>>("passive_scalar_systems");
    if (passive_scalar_system_names.size() != _passive_scalar_equation_relaxation.size())
      paramError("passive_scalar_equation_relaxation",
                 "The number of equation relaxation parameters does not match the number of "
                 "passive scalar equations!");
    if (passive_scalar_system_names.size() != _passive_scalar_absolute_tolerance.size())
      paramError("passive_scalar_absolute_tolerance",
                 "The number of absolute tolerances does not match the number of "
                 "passive scalar equations!");

    for (auto system_i : index_range(passive_scalar_system_names))
    {
      _passive_scalar_system_numbers.push_back(
          _problem.nlSysNum(passive_scalar_system_names[system_i]));
      _passive_scalar_systems.push_back(
          &_problem.getNonlinearSystemBase(_passive_scalar_system_numbers[system_i]));
    }
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

  _time = 0;
}

void
SIMPLE::checkDependentParameterError(const std::string & main_parameter,
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
SIMPLE::hasMultiAppError(const ExecFlagEnum & flags)
{
  bool has_error = false;
  for (const auto & flag : flags)
    if (_problem.hasMultiApps(flag))
    {
      _console << "\nCannot use SIMPLE solves with MultiApps set to execute on " << flag.name()
               << "!\nExiting...\n"
               << std::endl;
      has_error = true;
    }

  return has_error;
}

bool
SIMPLE::hasTransferError(const ExecFlagEnum & flags)
{
  for (const auto & flag : flags)
    if (_problem.getTransfers(flag, MultiAppTransfer::TO_MULTIAPP).size() ||
        _problem.getTransfers(flag, MultiAppTransfer::FROM_MULTIAPP).size() ||
        _problem.getTransfers(flag, MultiAppTransfer::BETWEEN_MULTIAPP).size())
    {
      _console << "\nCannot use SIMPLE solves with MultiAppTransfers set to execute on "
               << flag.name() << "!\nExiting...\n"
               << std::endl;
      return true;
    }

  return false;
}

void
SIMPLE::init()
{
  // check to make sure that we don't have any time kernels in this simulation (Steady State)
  for (const auto system : _momentum_systems)
    checkIntegrity(*system);

  checkIntegrity(_pressure_system);

  if (_has_energy_system)
  {
    checkIntegrity(*_energy_system);
    if (_has_solid_energy_system)
      checkIntegrity(*_solid_energy_system);
  }

  if (_has_passive_scalar_systems)
    for (const auto system : _passive_scalar_systems)
      checkIntegrity(*system);

  _problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _problem.initialSetup();

  // Fetch the segregated rhie-chow object and transfer some information about the momentum
  // system(s)
  _rc_uo = const_cast<INSFVRhieChowInterpolatorSegregated *>(
      &getUserObject<INSFVRhieChowInterpolatorSegregated>("rhie_chow_user_object"));
  _rc_uo->linkMomentumSystem(_momentum_systems, _momentum_system_numbers, _pressure_tag_id);

  // Initialize the face velocities in the RC object
  _rc_uo->initFaceVelocities();

  if (_pin_pressure)
    _pressure_pin_dof = findDoFID("pressure", getParam<Point>("pressure_pin_point"));
}

void
SIMPLE::relaxMatrix(SparseMatrix<Number> & matrix,
                    const Real relaxation_parameter,
                    NumericVector<Number> & diff_diagonal)
{
  // Zero the diagonal difference vector
  diff_diagonal = 0;

  // Get the diagonal of the matrix
  matrix.get_diagonal(diff_diagonal);

  // Create a copy of the diagonal for later use and cast it
  auto original_diagonal = diff_diagonal.clone();

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
  for (const auto row_i : make_range(matrix.row_start(), matrix.row_stop()))
  {
    std::vector<numeric_index_type> indices;
    std::vector<Real> values;
    matrix.get_row(row_i, indices, values);
    Real abs_sum = std::accumulate(
        values.cbegin(), values.cend(), 0.0, [](Real a, Real b) { return a + std::abs(b); });
    Real abs_diagonal = std::abs(diff_diagonal(row_i));
    Real new_diagonal = inverse_relaxation * std::max(abs_sum - abs_diagonal, abs_diagonal);
    diff_diagonal.set(row_i, new_diagonal);
  }
  diff_diagonal.close();

  // Time to modify the diagonal of the matrix
  for (const auto row_i : make_range(matrix.row_start(), matrix.row_stop()))
    matrix.set(row_i, row_i, diff_diagonal(row_i));
  matrix.close();

  // Finally, we can create (D*-D) vector which is used for the relaxation of the
  // right hand side later
  diff_diagonal.add(-1.0, *original_diagonal);
}

void
SIMPLE::relaxRightHandSide(NumericVector<Number> & rhs,
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
SIMPLE::computeNormalizationFactor(const NumericVector<Number> & solution,
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
SIMPLE::constrainSystem(SparseMatrix<Number> & mx,
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
    mx.set(dof_id, dof_id, 2 * diag);
  }
}

dof_id_type
SIMPLE::findDoFID(const VariableName & var_name, const Point & point)
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
SIMPLE::relaxSolutionUpdate(NonlinearSystemBase & system_in, Real relaxation_factor)
{
  // We will need the latest and the second latest solution for the relaxation
  NumericVector<Number> & solution = *(system_in.system().current_local_solution.get());
  NumericVector<Number> & solution_old = *(system_in.solutionPreviousNewton());

  // The relaxation is just u = lambda * u* + (1-lambda) u_old
  solution.scale(relaxation_factor);
  solution.add(1 - relaxation_factor, solution_old);
  solution.close();

  if (_print_fields)
  {
    _console << "Pressure solution" << std::endl;
    solution.print();
    _console << "Pressure solution old" << std::endl;
    solution_old.print();
  }

  // We will overwrite the old solution here
  solution_old = solution;
  system_in.setSolution(solution);
  system_in.residualSetup();
}

std::vector<Real>
SIMPLE::solveMomentumPredictor()
{
  // Temporary storage for the (flux-normalized) residuals form
  // different momentum components
  std::vector<Real> normalized_residuals;

  // We can create this here with the assumption that every momentum component has the same number
  // of dofs
  auto zero_solution = _momentum_systems[0]->system().current_local_solution->zero_clone();

  // Solve the momentum equations.
  // TO DO: These equations are VERY similar. If we can store the differences (things coming from
  // BCs for example) separately, it is enough to construct one matrix.
  for (const auto system_i : index_range(_momentum_systems))
  {
    _problem.setCurrentNonlinearSystem(_momentum_system_numbers[system_i]);

    // We will need the right hand side and the solution of the next component
    NonlinearImplicitSystem & momentum_system =
        libMesh::cast_ref<NonlinearImplicitSystem &>(_momentum_systems[system_i]->system());

    PetscLinearSolver<Real> & momentum_solver =
        libMesh::cast_ref<PetscLinearSolver<Real> &>(*momentum_system.get_linear_solver());

    NumericVector<Number> & solution = *(momentum_system.solution);
    NumericVector<Number> & rhs = *(momentum_system.rhs);
    SparseMatrix<Number> & mmat = *(momentum_system.matrix);

    auto diff_diagonal = solution.zero_clone();

    // We plug zero in this to get the system matrix and the right hand side of the linear problem
    _problem.computeResidualAndJacobian(*zero_solution, rhs, mmat);
    // Sadly, this returns -b so we multiply with -1
    rhs.scale(-1.0);

    // Still need to relax the right hand side with the same vector
    relaxMatrix(mmat, _momentum_equation_relaxation, *diff_diagonal);
    relaxRightHandSide(rhs, solution, *diff_diagonal);

    // The normalization factor depends on the right hand side so we need to recompute it for this
    // component
    Real norm_factor = computeNormalizationFactor(solution, mmat, rhs);

    // Very important, for deciding the convergence, we need the unpreconditioned
    // norms in the linear solve
    KSPSetNormType(momentum_solver.ksp(), KSP_NORM_UNPRECONDITIONED);
    // Solve this component. We don't update the ghosted solution yet, that will come at the end
    // of the corrector step. Also setting the linear tolerances and maximum iteration counts.
    _momentum_linear_control.real_valued_data["abs_tol"] = _momentum_l_abs_tol * norm_factor;
    momentum_solver.set_solver_configuration(_momentum_linear_control);

    // We solve the equation
    momentum_solver.solve(mmat, mmat, solution, rhs);
    momentum_system.update();

    // Save the normalized residual
    normalized_residuals.push_back(momentum_solver.get_initial_residual() / norm_factor);

    if (_print_fields)
    {
      _console << " matrix when we solve " << std::endl;
      mmat.print();
      _console << " rhs when we solve " << std::endl;
      rhs.print();
      _console << " velocity solution component " << system_i << std::endl;
      solution.print();
      _console << "Norm factor " << norm_factor << std::endl;
      _console << Moose::stringify(momentum_solver.get_initial_residual()) << std::endl;
    }
  }

  for (const auto system_i : index_range(_momentum_systems))
  {
    NonlinearImplicitSystem & momentum_system =
        libMesh::cast_ref<NonlinearImplicitSystem &>(_momentum_systems[system_i]->system());
    _momentum_systems[system_i]->setSolution(*(momentum_system.current_local_solution));
    _momentum_systems[system_i]->copySolutionsBackwards();
  }

  return normalized_residuals;
}

Real
SIMPLE::solvePressureCorrector()
{
  _problem.setCurrentNonlinearSystem(_pressure_sys_number);

  // We will need some members from the implicit nonlinear system
  NonlinearImplicitSystem & pressure_system =
      libMesh::cast_ref<NonlinearImplicitSystem &>(_pressure_system.system());

  // We will need the solution, the right hand side and the matrix
  NumericVector<Number> & current_local_solution = *(pressure_system.current_local_solution);
  NumericVector<Number> & solution = *(pressure_system.solution);
  SparseMatrix<Number> & mmat = *(pressure_system.matrix);
  NumericVector<Number> & rhs = *(pressure_system.rhs);

  // Fetch the linear solver from the system
  PetscLinearSolver<Real> & pressure_solver =
      libMesh::cast_ref<PetscLinearSolver<Real> &>(*pressure_system.get_linear_solver());

  // We need a zero vector to be able to emulate the Ax=b system by evaluating the
  // residual and jacobian. Unfortunately, this will leave us with the -b on the right hand side
  // so we correct it by multiplying it with (-1)
  auto zero_solution = current_local_solution.zero_clone();
  _problem.computeResidualAndJacobian(*zero_solution, rhs, mmat);
  rhs.scale(-1.0);

  if (_print_fields)
  {
    _console << "Pressure matrix" << std::endl;
    mmat.print();
  }

  // We compute the normalization factors based on the fluxes
  Real norm_factor = computeNormalizationFactor(solution, mmat, rhs);

  // We need the non-preconditioned norm to be consistent with the norm factor
  KSPSetNormType(pressure_solver.ksp(), KSP_NORM_UNPRECONDITIONED);

  // Setting the linear tolerances and maximum iteration counts
  _pressure_linear_control.real_valued_data["abs_tol"] = _pressure_l_abs_tol * norm_factor;
  pressure_solver.set_solver_configuration(_pressure_linear_control);

  if (_pin_pressure)
    constrainSystem(mmat, rhs, _pressure_pin_value, _pressure_pin_dof);

  pressure_solver.solve(mmat, mmat, solution, rhs);
  pressure_system.update();

  if (_print_fields)
  {
    _console << " rhs when we solve pressure " << std::endl;
    rhs.print();
    _console << " Pressure " << std::endl;
    solution.print();
    _console << "Norm factor " << norm_factor << std::endl;
  }

  _pressure_system.setSolution(current_local_solution);

  return pressure_solver.get_initial_residual() / norm_factor;
}

Real
SIMPLE::solveAdvectedSystem(const unsigned int system_num,
                            NonlinearSystemBase & system,
                            const Real relaxation_factor,
                            SolverConfiguration & solver_config,
                            const Real absolute_tol)
{
  _problem.setCurrentNonlinearSystem(system_num);

  // We will need some members from the implicit nonlinear system
  NonlinearImplicitSystem & ni_system =
      libMesh::cast_ref<NonlinearImplicitSystem &>(system.system());

  // We will need the solution, the right hand side and the matrix
  NumericVector<Number> & current_local_solution = *(ni_system.current_local_solution);
  NumericVector<Number> & solution = *(ni_system.solution);
  SparseMatrix<Number> & mmat = *(ni_system.matrix);
  NumericVector<Number> & rhs = *(ni_system.rhs);

  // We need a vector that stores the (diagonal_relaxed-original_diagonal) vector
  auto diff_diagonal = solution.zero_clone();

  // Fetch the linear solver from the system
  PetscLinearSolver<Real> & linear_solver =
      libMesh::cast_ref<PetscLinearSolver<Real> &>(*ni_system.get_linear_solver());

  // We need a zero vector to be able to emulate the Ax=b system by evaluating the
  // residual and jacobian. Unfortunately, this will leave us with the -b on the right hand side
  // so we correct it by multiplying it with (-1)
  auto zero_solution = current_local_solution.zero_clone();
  _problem.computeResidualAndJacobian(*zero_solution, rhs, mmat);
  rhs.scale(-1.0);

  // Go and relax the system matrix and the right hand side
  relaxMatrix(mmat, relaxation_factor, *diff_diagonal);
  relaxRightHandSide(rhs, solution, *diff_diagonal);

  if (_print_fields)
  {
    _console << system.name() << " system matrix" << std::endl;
    mmat.print();
  }

  // We compute the normalization factors based on the fluxes
  Real norm_factor = computeNormalizationFactor(solution, mmat, rhs);

  // We need the non-preconditioned norm to be consistent with the norm factor
  KSPSetNormType(linear_solver.ksp(), KSP_NORM_UNPRECONDITIONED);

  // Setting the linear tolerances and maximum iteration counts
  solver_config.real_valued_data["abs_tol"] = absolute_tol * norm_factor;
  linear_solver.set_solver_configuration(solver_config);

  // Solve the system and update current local solution
  linear_solver.solve(mmat, mmat, solution, rhs);
  ni_system.update();

  if (_print_fields)
  {
    _console << " rhs when we solve " << system.name() << std::endl;
    rhs.print();
    _console << system.name() << " solution " << std::endl;
    solution.print();
    _console << " Norm factor " << norm_factor << std::endl;
  }

  system.setSolution(current_local_solution);

  return linear_solver.get_initial_residual() / norm_factor;
}

Real
SIMPLE::solveSolidEnergySystem()
{
  _problem.setCurrentNonlinearSystem(_solid_energy_sys_number);

  // We will need some members from the implicit nonlinear system
  NonlinearImplicitSystem & se_system =
      libMesh::cast_ref<NonlinearImplicitSystem &>(_solid_energy_system->system());

  // We will need the solution, the right hand side and the matrix
  NumericVector<Number> & current_local_solution = *(se_system.current_local_solution);
  NumericVector<Number> & solution = *(se_system.solution);
  SparseMatrix<Number> & mat = *(se_system.matrix);
  NumericVector<Number> & rhs = *(se_system.rhs);

  // Fetch the linear solver from the system
  PetscLinearSolver<Real> & se_solver =
      libMesh::cast_ref<PetscLinearSolver<Real> &>(*se_system.get_linear_solver());

  // We need a zero vector to be able to emulate the Ax=b system by evaluating the
  // residual and jacobian. Unfortunately, this will leave us with the -b on the righ hand side
  // so we correct it by multiplying it with (-1)
  auto zero_solution = current_local_solution.zero_clone();
  _problem.computeResidualAndJacobian(*zero_solution, rhs, mat);
  rhs.scale(-1.0);

  if (_print_fields)
  {
    _console << "Solid energy matrix" << std::endl;
    mat.print();
  }

  // We compute the normalization factors based on the fluxes
  Real norm_factor = computeNormalizationFactor(solution, mat, rhs);

  // We need the non-preconditioned norm to be consistent with the norm factor
  KSPSetNormType(se_solver.ksp(), KSP_NORM_UNPRECONDITIONED);

  // Setting the linear tolerances and maximum iteration counts
  _solid_energy_linear_control.real_valued_data["abs_tol"] = _solid_energy_l_abs_tol * norm_factor;
  se_solver.set_solver_configuration(_solid_energy_linear_control);

  se_solver.solve(mat, mat, solution, rhs);
  se_system.update();

  if (_print_fields)
  {
    _console << " Solid energy rhs " << std::endl;
    rhs.print();
    _console << " Solid temperature " << std::endl;
    solution.print();
    _console << "Norm factor " << norm_factor << std::endl;
  }

  _solid_energy_system->setSolution(current_local_solution);

  return se_solver.get_initial_residual() / norm_factor;
}

void
SIMPLE::execute()
{
  if (_app.isRecovering())
  {
    _console << "\nCannot recover SIMPLE solves!\nExiting...\n" << std::endl;
    return;
  }

  if (_problem.adaptivity().isOn())
  {
    _console << "\nCannot use SIMPLE solves with mesh adaptivity!\nExiting...\n" << std::endl;
    return;
  }

  ExecFlagEnum disabled_flags;
  disabled_flags.addAvailableFlags(EXEC_TIMESTEP_BEGIN,
                                   EXEC_TIMESTEP_END,
                                   EXEC_INITIAL,
                                   EXEC_MULTIAPP_FIXED_POINT_BEGIN,
                                   EXEC_MULTIAPP_FIXED_POINT_END,
                                   EXEC_LINEAR,
                                   EXEC_NONLINEAR);

  if (hasMultiAppError(disabled_flags))
    return;
  if (hasTransferError(disabled_flags))
    return;

  _problem.timestepSetup();

  _time_step = 0;
  _problem.outputStep(EXEC_INITIAL);

  preExecute();

  _time_step = 1;

  preSolve();
  _problem.execute(EXEC_TIMESTEP_BEGIN);
  _problem.outputStep(EXEC_TIMESTEP_BEGIN);
  _problem.updateActiveObjects();

  // Dummy solver parameter file which is needed for switching petsc options
  SolverParams solver_params;
  solver_params._type = Moose::SolveType::ST_LINEAR;
  solver_params._line_search = Moose::LineSearchType::LS_NONE;

  if (_problem.shouldSolve())
  {
    // Initialize the quantities which matter in terms of the iteration
    unsigned int iteration_counter = 0;
    unsigned int no_systems =
        _momentum_systems.size() + 1 + _has_energy_system + _has_solid_energy_system;
    std::vector<Real> ns_residuals(no_systems, 1.0);
    std::vector<Real> ns_abs_tols(_momentum_systems.size(), _momentum_absolute_tolerance);
    ns_abs_tols.push_back(_pressure_absolute_tolerance);
    if (_has_energy_system)
    {
      ns_abs_tols.push_back(_energy_absolute_tolerance);
      if (_has_solid_energy_system)
        ns_abs_tols.push_back(_solid_energy_absolute_tolerance);
    }

    // Loop until converged or hit the maximum allowed iteration number
    while (iteration_counter < _num_iterations && !converged(ns_residuals, ns_abs_tols))
    {
      // We clear the caches in the momentum and pressure variables
      for (auto system_i : index_range(_momentum_systems))
        _momentum_systems[system_i]->residualSetup();
      _pressure_system.residualSetup();

      // If we solve for energy, we clear the caches there too
      if (_has_energy_system)
      {
        _energy_system->residualSetup();
        if (_has_solid_energy_system)
          _solid_energy_system->residualSetup();
      }

      iteration_counter++;

      // We set the preconditioner/controllable parameters through petsc options. Linear
      // tolerances will be overridden within the solver. In case of a segregated momentum
      // solver, we assume that every velocity component uses the same preconditioner
      Moose::PetscSupport::petscSetOptions(_momentum_petsc_options, solver_params);

      // Solve the momentum predictor step
      auto momentum_residual = solveMomentumPredictor();
      for (const auto system_i : index_range(momentum_residual))
        ns_residuals[system_i] = momentum_residual[system_i];

      // Compute the coupling fields between the momentum and pressure equations
      _rc_uo->computeHbyA(_print_fields);

      // We set the preconditioner/controllable parameters for the pressure equations through
      // petsc options. Linear tolerances will be overridden within the solver.
      Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);

      // Solve the pressure corrector
      ns_residuals[momentum_residual.size()] = solvePressureCorrector();
      // We need this to make sure we evaluate cell gradients for the nonorthogonal correction in
      // the face velocity update
      _pressure_system.residualSetup();

      // Compute the face velocity which is used in the advection terms
      _rc_uo->computeFaceVelocity();

      // Relax the pressure update for the next momentum predictor
      relaxSolutionUpdate(_pressure_system, _pressure_variable_relaxation);

      // Reconstruct the cell velocity as well to accelerate convergence
      _rc_uo->computeCellVelocity();

      // If we have an energy equation, solve it here. We assume the material properties in the
      // Navier-Stokes equations depend on temperature, therefore we can not solve for temperature
      // outside of the velocity-pressure loop
      if (_has_energy_system)
      {
        // We set the preconditioner/controllable parameters through petsc options. Linear
        // tolerances will be overridden within the solver.
        Moose::PetscSupport::petscSetOptions(_energy_petsc_options, solver_params);
        ns_residuals[momentum_residual.size() + 1] =
            solveAdvectedSystem(_energy_sys_number,
                                *_energy_system,
                                _energy_equation_relaxation,
                                _energy_linear_control,
                                _energy_l_abs_tol);

        if (_has_solid_energy_system)
        {
          // We set the preconditioner/controllable parameters through petsc options. Linear
          // tolerances will be overridden within the solver.
          Moose::PetscSupport::petscSetOptions(_energy_petsc_options, solver_params);
          ns_residuals[momentum_residual.size() + 2] = solveSolidEnergySystem();
        }
      }

      _console << "Iteration " << iteration_counter << " Initial residual norms:" << std::endl;
      for (auto system_i : index_range(_momentum_systems))
        _console << " Momentum equation:"
                 << (_momentum_systems.size() > 1
                         ? std::string(" Component ") + std::to_string(system_i + 1) +
                               std::string(" ")
                         : std::string(" "))
                 << COLOR_GREEN << ns_residuals[system_i] << COLOR_DEFAULT << std::endl;
      _console << " Pressure equation: " << COLOR_GREEN << ns_residuals[momentum_residual.size()]
               << COLOR_DEFAULT << std::endl;
      if (_has_energy_system)
      {
        _console << " Energy equation: " << COLOR_GREEN
                 << ns_residuals[momentum_residual.size() + 1] << COLOR_DEFAULT << std::endl;
        if (_has_solid_energy_system)
          _console << " Solid energy equation: " << COLOR_GREEN
                   << ns_residuals[momentum_residual.size() + 2] << COLOR_DEFAULT << std::endl;
      }
    }

    // Now we solve for the passive scalar equations, they should not influence the solution of the
    // system above. The reason why we need more than one iteration is due to the matrix relaxation
    // which can be used to stabilize the equations
    if (_has_passive_scalar_systems)
    {
      _console << " Passive Scalar Iteration " << iteration_counter << std::endl;

      // We set the options used by Petsc (preconditioners etc). We assume that every passive
      // scalar equation uses the same options for now.
      Moose::PetscSupport::petscSetOptions(_passive_scalar_petsc_options, solver_params);

      iteration_counter = 0;
      std::vector<Real> passive_scalar_residuals(_passive_scalar_systems.size(), 1.0);
      while (iteration_counter < _num_iterations &&
             !converged(passive_scalar_residuals, _passive_scalar_absolute_tolerance))
      {
        // We clear the caches in the passive scalar variables
        for (auto system_i : index_range(_passive_scalar_systems))
          _passive_scalar_systems[system_i]->residualSetup();

        iteration_counter++;

        // Solve the passive scalar equations
        for (auto system_i : index_range(_passive_scalar_systems))
          passive_scalar_residuals[system_i] =
              solveAdvectedSystem(_passive_scalar_system_numbers[system_i],
                                  *_passive_scalar_systems[system_i],
                                  _passive_scalar_equation_relaxation[system_i],
                                  _passive_scalar_linear_control,
                                  _passive_scalar_l_abs_tol);

        _console << "Iteration " << iteration_counter << " Initial residual norms:" << std::endl;
        for (auto system_i : index_range(_passive_scalar_systems))
          _console << _passive_scalar_systems[system_i]->name() << " " << COLOR_GREEN
                   << passive_scalar_residuals[system_i] << COLOR_DEFAULT << std::endl;
      }
    }
  }

  _time = _time_step;
  _problem.outputStep(EXEC_TIMESTEP_END);
  _time = _system_time;

  {
    TIME_SECTION("final", 1, "Executing Final Objects")
    _problem.execMultiAppTransfers(EXEC_FINAL, MultiAppTransfer::TO_MULTIAPP);
    _problem.execMultiAppTransfers(EXEC_FINAL, MultiAppTransfer::BETWEEN_MULTIAPP);
    _problem.execMultiApps(EXEC_FINAL);
    _problem.execMultiAppTransfers(EXEC_FINAL, MultiAppTransfer::FROM_MULTIAPP);
    _problem.finalizeMultiApps();
    _problem.postExecute();
    _problem.execute(EXEC_FINAL);
    _time = _time_step;
    _problem.outputStep(EXEC_FINAL);
    _time = _system_time;
  }

  postExecute();
}

bool
SIMPLE::converged(const std::vector<Real> & residuals, const std::vector<Real> & abs_tolerances)
{
  mooseAssert(residuals.size() == abs_tolerances.size(),
              "The number of residuals should (now " + std::to_string(residuals.size()) +
                  ") be the same as the number of tolerances (" +
                  std::to_string(abs_tolerances.size()) + ")!");

  bool converged = true;
  for (const auto system_i : index_range(residuals))
  {
    converged = converged && (residuals[system_i] < abs_tolerances[system_i]);
    if (!converged)
      return converged;
  }
  return converged;
}

void
SIMPLE::checkIntegrity(NonlinearSystemBase & system)
{
  // check to make sure that we don't have any time kernels in this simulation (Steady State)
  if (system.containsTimeKernel())
    mooseError("You have specified time kernels in your steady state simulation in system",
               system.name());
}
