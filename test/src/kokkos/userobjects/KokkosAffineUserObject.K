//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "KokkosAffineUserObject.h"

registerMooseObject("MooseTestApp", KokkosAffineUserObject);
registerVirtualKokkosUserObject(KokkosAffineUserObject, KokkosPolymorphicUserObjectBase);

InputParameters
KokkosAffineUserObject::validParams()
{
  InputParameters params = KokkosPolymorphicUserObjectBase::validParams();
  params.addRequiredParam<Real>("factor", "Linear factor applied to the input.");
  params.addRequiredParam<Real>("offset", "Offset added to the result.");
  return params;
}

KokkosAffineUserObject::KokkosAffineUserObject(const InputParameters & parameters)
  : KokkosPolymorphicUserObjectBase(parameters),
    _factor(getParam<Real>("factor")),
    _offset(getParam<Real>("offset"))
{
}
