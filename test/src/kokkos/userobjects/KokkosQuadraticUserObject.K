//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "KokkosQuadraticUserObject.h"

registerMooseObject("MooseTestApp", KokkosQuadraticUserObject);
registerVirtualKokkosUserObject(KokkosQuadraticUserObject, KokkosPolymorphicUserObjectBase);
registerVirtualKokkosUserObject(KokkosQuadraticUserObject, KokkosAffineUserObject);

InputParameters
KokkosQuadraticUserObject::validParams()
{
  InputParameters params = KokkosAffineUserObject::validParams();
  params.addRequiredParam<Real>("coefficient", "Coefficient applied to the squared input.");
  return params;
}

KokkosQuadraticUserObject::KokkosQuadraticUserObject(const InputParameters & parameters)
  : KokkosAffineUserObject(parameters), _coefficient(getParam<Real>("coefficient"))
{
}
