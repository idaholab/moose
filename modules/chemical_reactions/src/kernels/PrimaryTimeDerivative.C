/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PrimaryTimeDerivative.h"
#include "Material.h"

template<>
InputParameters validParams<PrimaryTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addParam<MaterialPropertyName>("porosity", "porosity", "The real material property (here is it a porosity) to use");
  return params;
}

PrimaryTimeDerivative::PrimaryTimeDerivative(const InputParameters & parameters) :
    TimeDerivative(parameters),
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


// DEPRECATED CONSTRUCTOR
PrimaryTimeDerivative::PrimaryTimeDerivative(const std::string & deprecated_name, InputParameters parameters) :
    TimeDerivative(deprecated_name, parameters),
    _porosity(getMaterialProperty<Real>("porosity"))
{}
