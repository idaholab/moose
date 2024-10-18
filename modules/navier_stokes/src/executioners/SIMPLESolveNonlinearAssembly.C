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
