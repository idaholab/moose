#include "SolidMechImplicitEuler.h"

template<>
InputParameters validParams<SolidMechImplicitEuler>()
{
  InputParameters params = validParams<SecondDerivativeImplicitEuler>();
  return params;
}

SolidMechImplicitEuler::SolidMechImplicitEuler(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :SecondDerivativeImplicitEuler(name,moose_system,parameters),
   _density(getMaterialProperty<Real>("density"))
{}

Real
SolidMechImplicitEuler::computeQpResidual()
  {
    return _density[_qp]*SecondDerivativeImplicitEuler::computeQpResidual();
  }

Real
SolidMechImplicitEuler::computeQpJacobian()
  {
    return _density[_qp]*SecondDerivativeImplicitEuler::computeQpJacobian();
  }
  
