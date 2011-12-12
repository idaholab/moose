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
  return NeumannBC::computeQpResidual();
}


Real
NeumannRZ::computeQpJacobian()
{
  return NeumannBC::computeQpJacobian();
}

Real
NeumannRZ::computeQpOffDiagJacobian( unsigned jvar )
{
  return NeumannBC::computeQpOffDiagJacobian( jvar );
}
