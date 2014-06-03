#include "PrimaryTimeDerivative.h"
#include "Material.h"

template<>
InputParameters validParams<PrimaryTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addParam<std::string>("porosity","porosity","The real material property (here is it a diffusivity) to use in this boundary condition");
  return params;
}

PrimaryTimeDerivative::PrimaryTimeDerivative(const std::string & name, InputParameters parameters) :
    TimeDerivative(name, parameters),
    _prop_name(getParam<std::string>("porosity")),
    _porosity(getMaterialProperty<Real>(_prop_name))
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
