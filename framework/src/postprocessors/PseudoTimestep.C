//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "PseudoTimestep.h"
#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"
#include "Transient.h"

// MOOSE includes
#include "libmesh/quadrature.h"
#include "libmesh/enum_norm_type.h"
#include "metaphysicl/raw_type.h"

#include <algorithm>
#include <limits>

registerMooseObject("NavierStokesApp", PseudoTimestep);

InputParameters
PseudoTimestep::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  MooseEnum method("SER=1 EXP=2 RDM=3");
  params.addRequiredParam<MooseEnum>("method", method, "The method used for CFL scaling");
  params.addParam<Real>("initial_dt", "Initial timestep");
  params.addParam<Real>("alpha", 1.0, "The parameter alpha used in the scaling of the timestep");
  params.addParam<Real>("max_dt", "The largest timestp allowed");
  params.addParam<unsigned int>(
      "iterations_window",
      1.0,
      "For how many iterations should the CFL be tracked (only applies to the SER method)");
  // Because this post-processor is meant to be used with PostprocessorDT, it
  // should be executed on initial (not included by default) and timestep end.
  params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_END, EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  params.addClassDescription("Computes a time step size to accelerate iterations");

  return params;
}

PseudoTimestep::PseudoTimestep(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _method(getParam<MooseEnum>("method")),
    _initial_dt(getParam<Real>("initial_dt")),
    _alpha(getParam<Real>("alpha")),
    _max_dt(getParam<Real>("max_dt")),
    _iterations_window(getParam<unsigned int>("iterations_window"))
{
}

void
PseudoTimestep::initialize()
{
  // start with the max
  _dt = std::numeric_limits<Real>::max();
}

Real
PseudoTimestep::getValue() const
{
  return _dt;
}

Real
PseudoTimestep::current_residual_norm() const
{
  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();
  Real var_res;
  Real res_norm = 0.0;
  for (unsigned int var_num = 0; var_num < nl.system().n_vars(); var_num++)
  {
    var_res = nl.system().calculate_norm(nl.getResidualNonTimeVector(), var_num, DISCRETE_L2);
    res_norm = res_norm + std::pow(var_res, 2);
  }
  res_norm = std::sqrt(res_norm);
  return res_norm;
}

void
PseudoTimestep::output_pseudo_timestep(Real & curr_dt, const ExecFlagType & exec_type)
{
  unsigned int curr_step = _fe_problem.timeStep();

  unsigned int steps_size = _iterations_step_sequence.size();
  unsigned int res_size = _residual_norms_sequence.size();

  Moose::out << std::endl;
  Moose::out << "***********************************************" << std::endl;
  Moose::out << "Current step " << curr_step << " executed at " << _current_execute_flag
             << " current timestep " << curr_dt << std::endl;

  Moose::out << "Iterations vector is of size " << steps_size << " , and residual norms is of size "
             << res_size << std::endl;
  Moose::out << "Current dt: " << _iterations_step_sequence[steps_size - 1]
             << " and residual: " << _residual_norms_sequence[res_size - 1] << std::endl;

  Moose::out << std::endl;
}

Real
PseudoTimestep::timestep_SER()
{
  unsigned int curr_step = _fe_problem.timeStep();

  unsigned int steps_size = _iterations_step_sequence.size();
  unsigned int res_size = _residual_norms_sequence.size();
  unsigned int prev_steps = std::min(_iterations_window, curr_step);

  Real factor = std::pow(_residual_norms_sequence[res_size - prev_steps] /
                             _residual_norms_sequence[res_size - 1],
                         _alpha);

  Real update_dt = _iterations_step_sequence[steps_size - 1] * factor;
  return update_dt;
}

Real
PseudoTimestep::timestep_RDM()
{
  unsigned int steps_size = _iterations_step_sequence.size();
  unsigned int res_size = _residual_norms_sequence.size();

  Real factor = (_residual_norms_sequence[res_size - 2] - _residual_norms_sequence[res_size - 1]) /
                _residual_norms_sequence[res_size - 2];

  Real update_dt = _iterations_step_sequence[steps_size - 1] * std::pow(_alpha, factor);
  return update_dt;
}

Real
PseudoTimestep::timestep_EXP()
{
  Real factor = std::pow(_alpha, _fe_problem.timeStep() - 1);

  Real update_dt = _initial_dt * factor;
  return update_dt;
}

void
PseudoTimestep::execute()
{
  Transient * transient = dynamic_cast<Transient *>(_app.getExecutioner());

  Real res_norm, var_res;
  Real curr_dt;
  Real update_dt;

  if (_current_execute_flag == EXEC_INITIAL)
  {
    _dt = _initial_dt;
  }

  if (_current_execute_flag == EXEC_TIMESTEP_END)
  {
    res_norm = current_residual_norm();
    _residual_norms_sequence.push_back(res_norm);

    curr_dt = transient->getDT();
    _iterations_step_sequence.push_back(curr_dt);

    _dt = curr_dt;
    if (_fe_problem.timeStep() > 1)
    {
      update_dt = curr_dt;
      switch (_method)
      {
        case 1:
          update_dt = timestep_SER();
          break;
        case 2:
          update_dt = timestep_EXP();
          break;
        case 3:
          update_dt = timestep_RDM();
          break;
      }
      _dt = std::min(_max_dt, update_dt);
    }
    output_pseudo_timestep(_dt, _current_execute_flag);
  }
}
