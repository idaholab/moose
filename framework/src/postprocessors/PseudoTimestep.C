//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PseudoTimestep.h"
#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"
#include "MathUtils.h"
#include "TransientBase.h"
#include "Restartable.h"
#include "libmesh/enum_norm_type.h"

registerMooseObject("MooseApp", PseudoTimestep);

InputParameters
PseudoTimestep::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  /// Enum that is used to select the timestep-selection method.
  MooseEnum method("SER RDM EXP", "SER");

  method.addDocumentation("SER", "Switched evolution relaxation");
  method.addDocumentation("EXP", "Exponential progression");
  method.addDocumentation("RDM", "Residual difference method");

  params.addRequiredParam<MooseEnum>(
      "method", method, "The method used for pseudotimestep timemarching");
  params.addRequiredRangeCheckedParam<Real>("initial_dt", "initial_dt > 0", "Initial timestep");
  params.addRequiredRangeCheckedParam<Real>(
      "alpha", "alpha > 0", "The parameter alpha used in the scaling of the timestep");
  params.addParam<Real>("max_dt", "The largest timestep allowed");
  params.addParam<unsigned int>(
      "iterations_window",
      "For how many iterations should the residual be tracked (only applies to the SER method)");
  // Because this post-processor is meant to be used with PostprocessorDT, it
  // should be executed on initial (not included by default) and timestep end.
  params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_END, EXEC_INITIAL};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  params.addClassDescription("Computes pseudo-time steps for obtaining steady-state solutions "
                             "through a pseudo transient process.");

  return params;
}

PseudoTimestep::PseudoTimestep(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _method(getParam<MooseEnum>("method").getEnum<PseudotimeMethod>()),
    _initial_dt(getParam<Real>("initial_dt")),
    _alpha(getParam<Real>("alpha")),
    _bound(isParamValid("max_dt")),
    _max_dt(_bound ? getParam<Real>("max_dt") : std::numeric_limits<Real>::max()),
    _residual_norms_sequence(declareRestartableData<std::vector<Real>>("residual_norms_sequence")),
    _iterations_step_sequence(declareRestartableData<std::vector<Real>>("iterations_step_sequence"))
{
  if (!_fe_problem.isTransient())
    mooseError("This pseudotimestepper can only be used if the steady state is computed via time "
               "integration.");

  if (_method == PseudotimeMethod::SER)
  {
    if (parameters.isParamValid("iterations_window"))
      _iterations_window = getParam<unsigned int>("iterations_window");
    else
      _iterations_window = 1;
  }
  else if (parameters.isParamValid("iterations_window"))
    mooseError("The iterations window can be only provided for the SER method.");
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
PseudoTimestep::currentResidualNorm() const
{
  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase(_sys.number());
  Real res_norm = 0.0;
  for (const auto var_num : make_range(nl.system().n_vars()))
  {
    auto var_res =
        nl.system().calculate_norm(nl.getResidualNonTimeVector(), var_num, libMesh::DISCRETE_L2);
    res_norm = res_norm + std::pow(var_res, 2);
  }
  res_norm = std::sqrt(res_norm);
  return res_norm;
}

void
PseudoTimestep::outputPseudoTimestep(Real curr_time) const
{
  unsigned int curr_step = _fe_problem.timeStep();
  Real step_dt = _fe_problem.dt();

  unsigned int res_size = _residual_norms_sequence.size();

  _console << "Current step " << curr_step << " and current dt " << step_dt
           << " for  steady state residual " << _residual_norms_sequence[res_size - 1]
           << " at time " << curr_time << std::endl;
}

Real
PseudoTimestep::timestepSER() const
{
  const unsigned int curr_step = _fe_problem.timeStep();

  const unsigned int steps_size = _iterations_step_sequence.size();
  const unsigned int res_size = _residual_norms_sequence.size();
  const unsigned int prev_steps = std::min(_iterations_window + 1, curr_step);

  const Real factor = std::pow(_residual_norms_sequence[res_size - prev_steps] /
                                   _residual_norms_sequence[res_size - 1],
                               _alpha);

  const Real update_dt = _iterations_step_sequence[steps_size - 1] * factor;
  return update_dt;
}

Real
PseudoTimestep::timestepRDM() const
{
  const unsigned int res_size = _residual_norms_sequence.size();
  const unsigned int steps_size = _iterations_step_sequence.size();

  Real exponent = 0.0;

  if (_residual_norms_sequence[res_size - 1] < _residual_norms_sequence[res_size - 2])
    exponent = (_residual_norms_sequence[res_size - 2] - _residual_norms_sequence[res_size - 1]) /
               _residual_norms_sequence[res_size - 2];
  const Real update_dt = _iterations_step_sequence[steps_size - 1] * std::pow(_alpha, exponent);
  return update_dt;
}

Real
PseudoTimestep::timestepEXP() const
{
  const Real factor = MathUtils::pow(_alpha, _fe_problem.timeStep() - 1);

  const Real update_dt = _initial_dt * factor;
  return update_dt;
}

void
PseudoTimestep::execute()
{
  TransientBase * transient = dynamic_cast<TransientBase *>(_app.getExecutioner());

  Real res_norm;
  Real curr_dt;
  Real update_dt;

  if (_current_execute_flag == EXEC_INITIAL)
    _dt = _initial_dt;

  // at the end of each timestep call the postprocessor to set values for dt
  if (_current_execute_flag == EXEC_TIMESTEP_END)
  {
    res_norm = currentResidualNorm();
    _residual_norms_sequence.push_back(res_norm);

    curr_dt = transient->getDT();
    _iterations_step_sequence.push_back(curr_dt);

    _dt = curr_dt;

    // since some of the residual methods require previous residuals the pseudo timesteppers
    // start at the second step in the computation
    if (_fe_problem.timeStep() > 1)
    {
      update_dt = curr_dt;
      switch (_method)
      {
        case PseudotimeMethod::SER:
          update_dt = timestepSER();
          break;
        case PseudotimeMethod::EXP:
          update_dt = timestepEXP();
          break;
        case PseudotimeMethod::RDM:
          update_dt = timestepRDM();
          break;
      }
      if (_bound)
        _dt = std::min(_max_dt, update_dt);
      else
        _dt = update_dt;
    }
    outputPseudoTimestep(_fe_problem.time());
  }
}
