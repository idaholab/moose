//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowChannel1PhaseConvergence.h"

registerMooseObject("ThermalHydraulicsApp", FlowChannel1PhaseConvergence);

InputParameters
FlowChannel1PhaseConvergence::validParams()
{
  InputParameters params = MultiPostprocessorConvergence::validParams();

  params.addClassDescription("Assesses convergence of a FlowChannel1Phase component.");

  params.addRequiredParam<PostprocessorName>("p_rel_step", "Pressure relative step post-processor");
  params.addRequiredParam<PostprocessorName>("T_rel_step",
                                             "Temperature relative step post-processor");
  params.addRequiredParam<PostprocessorName>("vel_rel_step",
                                             "Velocity relative step post-processor");

  params.addRequiredParam<PostprocessorName>(
      "mass_res", "Normalized residual norm post-processor for the mass equation [1/s]");
  params.addRequiredParam<PostprocessorName>(
      "momentum_res", "Normalized residual norm post-processor for the momentum equation [1/s]");
  params.addRequiredParam<PostprocessorName>(
      "energy_res", "Normalized residual norm post-processor for the energy equation [1/s]");

  params.addRequiredParam<Real>("p_rel_step_tol", "Relative step tolerance for pressure");
  params.addRequiredParam<Real>("T_rel_step_tol", "Relative step tolerance for temperature");
  params.addRequiredParam<Real>("vel_rel_step_tol", "Relative step tolerance for velocity");

  params.addRequiredParam<Real>("mass_res_tol",
                                "Normalized residual tolerance for the mass equation [1/s]");
  params.addRequiredParam<Real>("momentum_res_tol",
                                "Normalized residual tolerance for the momentum equation [1/s]");
  params.addRequiredParam<Real>("energy_res_tol",
                                "Normalized residual tolerance for the energy equation [1/s]");

  return params;
}

FlowChannel1PhaseConvergence::FlowChannel1PhaseConvergence(const InputParameters & parameters)
  : MultiPostprocessorConvergence(parameters),
    _p_rel_step(getPostprocessorValue("p_rel_step")),
    _T_rel_step(getPostprocessorValue("T_rel_step")),
    _vel_rel_step(getPostprocessorValue("vel_rel_step")),
    _mass_res(getPostprocessorValue("mass_res")),
    _momentum_res(getPostprocessorValue("momentum_res")),
    _energy_res(getPostprocessorValue("energy_res")),
    _p_rel_step_tol(getParam<Real>("p_rel_step_tol")),
    _T_rel_step_tol(getParam<Real>("T_rel_step_tol")),
    _vel_rel_step_tol(getParam<Real>("vel_rel_step_tol")),
    _mass_res_tol(getParam<Real>("mass_res_tol")),
    _momentum_res_tol(getParam<Real>("momentum_res_tol")),
    _energy_res_tol(getParam<Real>("energy_res_tol"))
{
}

std::vector<std::tuple<std::string, Real, Real>>
FlowChannel1PhaseConvergence::getDescriptionErrorToleranceTuples() const
{
  return {{"step: p      ", _p_rel_step, _p_rel_step_tol},
          {"step: T      ", _T_rel_step, _T_rel_step_tol},
          {"step: vel    ", _vel_rel_step, _vel_rel_step_tol},
          {"res: mass    ", _mass_res, _mass_res_tol},
          {"res: momentum", _momentum_res, _momentum_res_tol},
          {"res: energy  ", _energy_res, _energy_res_tol}};
}
