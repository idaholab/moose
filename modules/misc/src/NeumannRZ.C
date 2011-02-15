#include "NeumannRZ.h"

template<>
InputParameters validParams<NeumannRZ>()
{
  return validParams<NeumannBC>();
}

NeumannRZ::NeumannRZ(const std::string & name, InputParameters parameters)
  :NeumannBC(name, parameters)
{}

Real
NeumannRZ::computeQpResidual()
{
  return 2 * M_PI * _q_point[_qp](0) * NeumannBC::computeQpResidual();
}


Real
NeumannRZ::computeQpJacobian()
{
  return 2 * M_PI * _q_point[_qp](0) * NeumannBC::computeQpJacobian();
}
