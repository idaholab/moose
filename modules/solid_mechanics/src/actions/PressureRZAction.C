#include "PressureRZAction.h"

template<>
InputParameters validParams<PressureRZAction>()
{
  InputParameters params = validParams<PressureAction>();
  return params;
}

PressureRZAction::PressureRZAction(const std::string & name, InputParameters params) :
  PressureAction(name, params)
{
  _kernel_name = "PressureRZ";
  _use_displaced_mesh = false;
}
