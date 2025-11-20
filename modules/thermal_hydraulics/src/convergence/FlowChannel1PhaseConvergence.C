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
  InputParameters params = Convergence::validParams();

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
  : Convergence(parameters),
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

Convergence::MooseConvergenceStatus
FlowChannel1PhaseConvergence::checkConvergence(unsigned int /*iter*/)
{
  bool all_converged = true;
  std::ostringstream oss;
  oss << "\n";

  std::vector<std::tuple<std::string, Real, Real>> step_err_tol_tuples{
      {"p  ", _p_rel_step, _p_rel_step_tol},
      {"T  ", _T_rel_step, _T_rel_step_tol},
      {"vel", _vel_rel_step, _vel_rel_step_tol}};
  oss << "  Step errors:\n";
  for (const auto & err_tol_tuple : step_err_tol_tuples)
  {
    const Real err = std::get<1>(err_tol_tuple);
    const Real tol = std::get<2>(err_tol_tuple);
    if (std::abs(err) > tol)
      all_converged = false;

    const std::string desc = std::get<0>(err_tol_tuple);
    oss << comparisonLine(desc, err, tol);
  }

  const std::vector<std::tuple<std::string, Real, Real>> res_err_tol_tuples{
      {"mass    ", _mass_res, _mass_res_tol},
      {"momentum", _momentum_res, _momentum_res_tol},
      {"energy  ", _energy_res, _energy_res_tol}};
  oss << "  Residual errors:\n";
  for (const auto & err_tol_tuple : res_err_tol_tuples)
  {
    const Real err = std::get<1>(err_tol_tuple);
    const Real tol = std::get<2>(err_tol_tuple);
    if (std::abs(err) > tol)
      all_converged = false;

    const std::string desc = std::get<0>(err_tol_tuple);
    oss << comparisonLine(desc, err, tol);
  }

  verboseOutput(oss);

  if (all_converged)
    return Convergence::MooseConvergenceStatus::CONVERGED;
  else
    return Convergence::MooseConvergenceStatus::ITERATING;
}

std::string
FlowChannel1PhaseConvergence::comparisonLine(const std::string & description,
                                             Real err,
                                             Real tol) const
{
  std::string color, compare_str;
  if (std::abs(err) > tol)
  {
    color = COLOR_RED;
    compare_str = ">";
  }
  else
  {
    color = COLOR_GREEN;
    compare_str = "<";
  }

  std::ostringstream oss;
  oss << "    " << description << ": " << color << std::abs(err) << " " << compare_str << " " << tol
      << COLOR_DEFAULT << "\n";
  return oss.str();
}
