/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "MaskedBodyForce.h"

// MOOSE
#include "Function.h"

template<>
InputParameters validParams<MaskedBodyForce>()
{
  InputParameters params = validParams<BodyForce>();
  params.addClassDescription("Kernel that defines a body force modified by a material mask");
  params.addParam<std::string>("mask", "Material property defining the mask");
  return params;
}

MaskedBodyForce::MaskedBodyForce(const std::string & name, InputParameters parameters) :
    BodyForce(name, parameters),
    _mask_property_name(getParam<std::string>("mask")),
    _mask(getMaterialProperty<Real>(_mask_property_name))
{
}

Real
MaskedBodyForce::computeQpResidual()
{
  return BodyForce::computeQpResidual()*_mask[_qp];
}
