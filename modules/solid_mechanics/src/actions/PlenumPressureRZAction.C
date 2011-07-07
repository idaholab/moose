#include "PlenumPressureRZAction.h"

template<>
InputParameters validParams<PlenumPressureRZAction>()
{
  InputParameters params = validParams<PlenumPressureAction>();
  return params;
}

PlenumPressureRZAction::PlenumPressureRZAction(const std::string & name, InputParameters params) :
  PlenumPressureAction(name, params)
{
  _kernel_name = "PlenumPressureRZ";
  _use_displaced_mesh = false;
}
