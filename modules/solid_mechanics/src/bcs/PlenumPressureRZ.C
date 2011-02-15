#include "PlenumPressureRZ.h"

#include "Function.h"
#include "Moose.h"

template<>
InputParameters validParams<PlenumPressureRZ>()
{
  InputParameters params = validParams<PlenumPressure>();
  return params;
}

PlenumPressureRZ::PlenumPressureRZ(const std::string & name, InputParameters parameters)
  :PlenumPressure(name, parameters)
{
}

Real
PlenumPressureRZ::computeQpResidual()
{
  return 2 * M_PI * _q_point[_qp](0) * PlenumPressure::computeQpResidual();
}
