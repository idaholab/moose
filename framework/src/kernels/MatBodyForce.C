//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatBodyForce.h"
#include "Function.h"

registerMooseObject("MooseApp", MatBodyForce);
registerMooseObject("MooseApp", ADMatBodyForce);

template <bool is_ad, class Parent>
InputParameters
MatBodyForceTempl<is_ad, Parent>::validParams()
{
  InputParameters params = Parent::validParams();
  params.addClassDescription("Kernel that defines a body force modified by a material property");
  params.addRequiredParam<MaterialPropertyName>("material_property",
                                                "Material property defining the property");
  return params;
}

template <bool is_ad, class Parent>
MatBodyForceTempl<is_ad, Parent>::MatBodyForceTempl(const InputParameters & parameters)
  : Parent(parameters),
    _property(this->template getGenericMaterialProperty<Real, is_ad>("material_property")),
    _v_name(_var.name())
{
}

template <bool is_ad, class Parent>
GenericReal<is_ad>
MatBodyForceTempl<is_ad, Parent>::computeQpResidual()
{
  return Parent::computeQpResidual() * _property[_qp];
}

MatBodyForce::MatBodyForce(const InputParameters & parameters)
  : MatBodyForceParent(parameters),
    _dpropertydv(getMaterialPropertyDerivative<Real>("property", _v_name)),
    _dpropertydarg(_n_args)
{
  // Get derivatives of property wrt coupled variables
  for (unsigned int i = 0; i < _n_args; ++i)
    _dpropertydarg[i] = &getMaterialPropertyDerivative<Real>("property", i);
}

void
MatBodyForce::initialSetup()
{
  validateNonlinearCoupling<Real>("property");
}

Real
MatBodyForce::computeQpJacobian()
{
  return _dpropertydv[_qp] * BodyForce::computeQpResidual() * _phi[_j][_qp];
}

Real
MatBodyForce::computeQpOffDiagJacobian(unsigned int jvar)
{
  const unsigned int cvar = mapJvarToCvar(jvar);
  return (*_dpropertydarg[cvar])[_qp] * BodyForce::computeQpResidual() * _phi[_j][_qp];
}

template class MatBodyForceTempl<true, ADBodyForce>;
