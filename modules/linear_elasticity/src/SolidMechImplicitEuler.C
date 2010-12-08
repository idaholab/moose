#include "SolidMechImplicitEuler.h"

template<>
InputParameters validParams<SolidMechImplicitEuler>()
{
  InputParameters params = validParams<SecondDerivativeImplicitEuler>();
  return params;
}

SolidMechImplicitEuler::SolidMechImplicitEuler(const std::string & name, InputParameters parameters)
  :SecondDerivativeImplicitEuler(name,parameters),
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
  
