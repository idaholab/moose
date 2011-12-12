#include "PressureRZ.h"

template<>
InputParameters validParams<PressureRZ>()
{
  InputParameters params = validParams<Pressure>();
  params.set<bool>("use_displaced_mesh") = false;
  return params;
}

PressureRZ::PressureRZ(const std::string & name, InputParameters parameters)
  :Pressure(name, parameters)
{
}

Real
PressureRZ::computeQpResidual()
{
  return Pressure::computeQpResidual();
}
