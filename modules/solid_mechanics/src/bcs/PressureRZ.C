#include "PressureRZ.h"

#include "Function.h"
#include "Moose.h"

template<>
InputParameters validParams<PressureRZ>()
{
  InputParameters params = validParams<Pressure>();
  return params;
}

PressureRZ::PressureRZ(const std::string & name, InputParameters parameters)
  :Pressure(name, parameters)
{
}

Real
PressureRZ::computeQpResidual()
{
  return 2 * M_PI * _q_point[_qp](0) * Pressure::computeQpResidual();
}
