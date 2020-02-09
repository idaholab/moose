//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceQpMaterialPropertyRealUO.h"
#include "MooseMesh.h"
registerMooseObject("MooseApp", InterfaceQpMaterialPropertyRealUO);

defineLegacyParams(InterfaceQpMaterialPropertyRealUO);

InputParameters
InterfaceQpMaterialPropertyRealUO::validParams()
{
  InputParameters params = InterfaceQpMaterialPropertyBaseUserObject<Real>::validParams();
  params.addClassDescription("Computes the value or rate of a Real Material property across an "
                             "interface. The value or rate is computed according to the provided "
                             "interface_value_type parameter");
  return params;
}

InterfaceQpMaterialPropertyRealUO::InterfaceQpMaterialPropertyRealUO(
    const InputParameters & parameters)
  : InterfaceQpMaterialPropertyBaseUserObject<Real>(parameters)

{
}

Real
InterfaceQpMaterialPropertyRealUO::computeRealValueMaster(const unsigned int qp)
{
  if (_compute_rate)
  {
    if (_dt != 0)
      return (_prop[qp] - (*_prop_old)[qp]) / _dt;
    else
      return 0;
  }
  else
    return _prop[qp];
}

Real
InterfaceQpMaterialPropertyRealUO::computeRealValueSlave(const unsigned int qp)
{
  if (_compute_rate)
  {
    if (_dt != 0)
      return (_prop_neighbor[qp] - (*_prop_neighbor_old)[qp]) / _dt;
    else
      return 0;
  }
  else
    return _prop_neighbor[qp];
}
