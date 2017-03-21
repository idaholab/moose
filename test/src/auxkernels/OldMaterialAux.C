/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "OldMaterialAux.h"

template <>
InputParameters
validParams<OldMaterialAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<MaterialPropertyName>(
      "property_name", "The name of the material property to capture old and older values from");
  return params;
}

OldMaterialAux::OldMaterialAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _old(getMaterialPropertyOld<Real>("property_name")),
    _older(getMaterialPropertyOlder<Real>("property_name"))
{
}

Real
OldMaterialAux::computeValue()
{
  return _old[_qp] + _older[_qp];
}
