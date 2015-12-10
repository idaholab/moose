/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HEVPStrengthUOBase.h"

template<>
InputParameters validParams<HEVPStrengthUOBase>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addParam<std::string>("intvar_prop_name", "Name of internal variable property to calculate material resistance: Same as internal variable user object");
  params.addClassDescription("User Object to compute material resistance");
  return params;
}

HEVPStrengthUOBase::HEVPStrengthUOBase(const InputParameters & parameters) :
    ElementUserObject(parameters),
    _intvar_prop_name(getParam<std::string>("intvar_prop_name")),
    _intvar(getMaterialPropertyByName<Real>(_intvar_prop_name))
{
}
