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
  return ConvectiveFluxBC::computeQpResidual();
}

Real
ConvectiveFluxRZ::computeQpJacobian()
{
  return ConvectiveFluxBC::computeQpJacobian();
}

Real
ConvectiveFluxRZ::computeQpOffDiagJacobian( unsigned jvar )
{
  return ConvectiveFluxBC::computeQpOffDiagJacobian( jvar );
}
