#include "PrimaryImplicitEuler.h"
#include "Material.h"

template<>
InputParameters validParams<PrimaryImplicitEuler>()
{
  InputParameters params = validParams<ImplicitEuler>();
  return params;
}

PrimaryImplicitEuler::PrimaryImplicitEuler(const std::string & name, InputParameters parameters)
  :ImplicitEuler(name,parameters),
   _porosity(getMaterialProperty<Real>("porosity"))
{}

Real
PrimaryImplicitEuler::computeQpResidual()
{
  return _porosity[_qp]*ImplicitEuler::computeQpResidual();
}

Real
PrimaryImplicitEuler::computeQpJacobian()
{
  return _porosity[_qp]*ImplicitEuler::computeQpJacobian();
}
