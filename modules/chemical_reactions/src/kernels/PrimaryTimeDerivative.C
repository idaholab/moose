#include "PrimaryTimeDerivative.h"
#include "Material.h"

template<>
InputParameters validParams<PrimaryTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
  return params;
}

PrimaryTimeDerivative::PrimaryTimeDerivative(const std::string & name, InputParameters parameters) :
    TimeDerivative(name, parameters),
    _porosity(getMaterialProperty<Real>("porosity"))
{}

Real
PrimaryTimeDerivative::computeQpResidual()
{
  return _porosity[_qp]*TimeDerivative::computeQpResidual();
}

Real
PrimaryTimeDerivative::computeQpJacobian()
{
  return _porosity[_qp]*TimeDerivative::computeQpJacobian();
}

Real PrimaryTimeDerivative::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.0;
}
