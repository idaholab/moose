#include "ConvectiveFluxRZ.h"

template<>
InputParameters validParams<ConvectiveFluxRZ>()
{
  InputParameters params = validParams<ConvectiveFluxBC>();
  return params;
}

ConvectiveFluxRZ::ConvectiveFluxRZ(const std::string & name, InputParameters parameters)
 :ConvectiveFluxBC(name, parameters)
{}


Real
ConvectiveFluxRZ::computeQpResidual()
{
  return 2 * M_PI * _q_point[_qp](0) * ConvectiveFluxBC::computeQpResidual();
}

Real
ConvectiveFluxRZ::computeQpJacobian()
{
  return 2 * M_PI * _q_point[_qp](0) * ConvectiveFluxBC::computeQpJacobian();
}
