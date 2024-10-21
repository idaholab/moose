//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SIMPLESolveNonlinearAssembly.h"
#include "FEProblem.h"
#include "SegregatedSolverUtils.h"

InputParameters
SIMPLESolveNonlinearAssembly::validParams()
{
  InputParameters params = SIMPLESolveBase::validParams();

  /*
   * The names of the different systems in the segregated solver
   */
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

  params.addParamNamesToGroup("energy_equation_relaxation passive_scalar_equation_relaxation "
                              "turbulence_equation_relaxation",
                              "Relaxation");

  /*
   * Petsc options for every equations in the system
   */
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

  params.addParamNamesToGroup(
      "energy_absolute_tolerance "
      "solid_energy_absolute_tolerance passive_scalar_absolute_tolerance "
      "turbulence_absolute_tolerance",
      "Iteration Control");
  /*
   * Linear iteration tolerances for the different equations
   */
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
      "solid_energy_l_tol solid_energy_l_abs_tol solid_energy_l_max_its "
      "energy_l_tol energy_l_abs_tol energy_l_max_its passive_scalar_l_tol "
      "passive_scalar_l_abs_tol passive_scalar_l_max_its turbulence_l_tol "
      "turbulence_l_abs_tol turbulence_l_max_its",
      "Linear Iteration Control");

  return params;
}

SIMPLESolveNonlinearAssembly::SIMPLESolveNonlinearAssembly(Executioner & ex)
  : SIMPLESolveBase(ex),
    _pressure_sys_number(_problem.nlSysNum(getParam<SolverSystemName>("pressure_system"))),
    _pressure_system(_problem.getNonlinearSystemBase(_pressure_sys_number)),
    _has_energy_system(isParamValid("energy_system")),
    _has_solid_energy_system(_has_energy_system && isParamValid("solid_energy_system")),
    _has_passive_scalar_systems(isParamValid("passive_scalar_systems")),
    _has_turbulence_systems(isParamValid("turbulence_systems")),
    _energy_sys_number(_has_energy_system
                           ? _problem.nlSysNum(getParam<SolverSystemName>("energy_system"))
                           : libMesh::invalid_uint),
    _energy_system(_has_energy_system ? &_problem.getNonlinearSystemBase(_energy_sys_number)
                                      : nullptr),
    _energy_equation_relaxation(getParam<Real>("energy_equation_relaxation")),
    _energy_l_abs_tol(getParam<Real>("energy_l_abs_tol")),
    _solid_energy_sys_number(
        _has_solid_energy_system
            ? _problem.nlSysNum(getParam<SolverSystemName>("solid_energy_system"))
            : libMesh::invalid_uint),
    _solid_energy_system(_has_solid_energy_system
                             ? &_problem.getNonlinearSystemBase(_solid_energy_sys_number)
                             : nullptr),
    _solid_energy_l_abs_tol(getParam<Real>("solid_energy_l_abs_tol")),
    _passive_scalar_system_names(getParam<std::vector<SolverSystemName>>("passive_scalar_systems")),
    _passive_scalar_equation_relaxation(
        getParam<std::vector<Real>>("passive_scalar_equation_relaxation")),
    _passive_scalar_l_abs_tol(getParam<Real>("passive_scalar_l_abs_tol")),
    _turbulence_system_names(getParam<std::vector<SolverSystemName>>("turbulence_systems")),
    _turbulence_equation_relaxation(getParam<std::vector<Real>>("turbulence_equation_relaxation")),
    _turbulence_field_min_limit(getParam<std::vector<Real>>("turbulence_field_min_limit")),
    _turbulence_l_abs_tol(getParam<Real>("turbulence_l_abs_tol")),
    _energy_absolute_tolerance(getParam<Real>("energy_absolute_tolerance")),
    _solid_energy_absolute_tolerance(getParam<Real>("solid_energy_absolute_tolerance")),
    _passive_scalar_absolute_tolerance(
        getParam<std::vector<Real>>("passive_scalar_absolute_tolerance")),
    _turbulence_absolute_tolerance(getParam<std::vector<Real>>("turbulence_absolute_tolerance")),
    _pressure_tag_name(getParam<TagName>("pressure_gradient_tag")),
    _pressure_tag_id(_problem.addVectorTag(_pressure_tag_name))
{
  // We fetch the system numbers for the momentum components plus add vectors
  // for removing the contribution from the pressure gradient terms.
  for (auto system_i : index_range(_momentum_system_names))
  {
    _momentum_system_numbers.push_back(_problem.nlSysNum(_momentum_system_names[system_i]));
    _momentum_systems.push_back(
        &_problem.getNonlinearSystemBase(_momentum_system_numbers[system_i]));
    _momentum_systems[system_i]->addVector(_pressure_tag_id, false, ParallelType::PARALLEL);
  }

  if (_has_passive_scalar_systems)
    for (auto system_i : index_range(_passive_scalar_system_names))
    {
      _passive_scalar_system_numbers.push_back(
          _problem.nlSysNum(_passive_scalar_system_names[system_i]));
      _passive_scalar_systems.push_back(
          &_problem.getNonlinearSystemBase(_passive_scalar_system_numbers[system_i]));
    }

  if (_has_turbulence_systems)
    for (auto system_i : index_range(_turbulence_system_names))
    {
      _turbulence_system_numbers.push_back(_problem.nlSysNum(_turbulence_system_names[system_i]));
      _turbulence_systems.push_back(
          &_problem.getNonlinearSystemBase(_turbulence_system_numbers[system_i]));
    }
}

void
SIMPLESolveNonlinearAssembly::linkRhieChowUserObject()
{
  // Fetch the segregated rhie-chow object and transfer some information about the momentum
  // system(s)
  _rc_uo = const_cast<INSFVRhieChowInterpolatorSegregated *>(
      &getUserObject<INSFVRhieChowInterpolatorSegregated>("rhie_chow_user_object"));
  _rc_uo->linkMomentumSystem(_momentum_systems, _momentum_system_numbers, _pressure_tag_id);

  // Initialize the face velocities in the RC object
  _rc_uo->initFaceVelocities();
}

std::vector<std::pair<unsigned int, Real>>
SIMPLESolveNonlinearAssembly::solveMomentumPredictor()
{
  mooseError("Should not get here yet");
}

std::pair<unsigned int, Real>
SIMPLESolveNonlinearAssembly::solvePressureCorrector()
{
  mooseError("Should not get here yet");
}

bool
SIMPLESolveNonlinearAssembly::solve()
{
  mooseError("Should not get here yet");
}
