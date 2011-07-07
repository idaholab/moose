#include "PlenumPressureRZ.h"

template<>
InputParameters validParams<PlenumPressureRZ>()
{
  InputParameters params = validParams<PlenumPressure>();
  params.set<bool>("use_displaced_mesh") = false;
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
