#include "RunningTimeAverageAux.h"

registerMooseObject("MooseApp", RunningTimeAverageAux);

InputParameters
RunningTimeAverageAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Running time-weighted mean of a functor, starting at a given simulation time.");

  params.addRequiredParam<MooseFunctorName>(
      "functor", "The variable or functor to time-average.");
  params.addRequiredParam<Real>("start_time", "Simulation time at which averaging starts.");
  return params;
}

RunningTimeAverageAux::RunningTimeAverageAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _functor(getFunctor<Real>("functor")),
    _mean_old(uOld()),
    _start_time(getParam<Real>("start_time")),
    _averaging(declareRestartableData<bool>("averaging", false)),
    _t_start(declareRestartableData<Real>("t_start", 0.0))
{
  if (isNodal())
    paramError("variable", "The averaged variable must be elemental (CONSTANT MONOMIAL).");
}

void
RunningTimeAverageAux::timestepSetup()
{
  // Called once per time step, after time has advanced
  if (!_averaging && _t >= _start_time)
  {
    _averaging = true;
    _t_start = _t - _dt; // include the current step in the average
  }
}

Real
RunningTimeAverageAux::computeValue()
{
  const Moose::ElemArg elem_arg{_current_elem, false};
  const Real u = _functor(elem_arg, Moose::currentState());

  if (!_averaging)
    return u;

  const Real T = _t - _t_start; // total averaging time including this step
  return _mean_old[_qp] + (u - _mean_old[_qp]) * _dt / T;
}
