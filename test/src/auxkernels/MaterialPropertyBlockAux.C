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

#include "MaterialPropertyBlockAux.h"

#include "Assembly.h"

template <>
InputParameters
validParams<MaterialPropertyBlockAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addParam<MaterialPropertyName>("mat_prop_name",
                                        "The material property to check where it is defined");
  return params;
}

MaterialPropertyBlockAux::MaterialPropertyBlockAux(const InputParameters & params)
  : AuxKernel(params),
    _mat_prop_name(getParam<MaterialPropertyName>("mat_prop_name")),
    _has_prop(false)
{
  if (isNodal())
    mooseError("This AuxKernel only supports Elemental fields");
}

void
MaterialPropertyBlockAux::subdomainSetup()
{
  _has_prop = _subproblem.hasBlockMaterialProperty(_assembly.currentSubdomainID(), _mat_prop_name);
}

Real
MaterialPropertyBlockAux::computeValue()
{
  if (_has_prop)
    return 1;
  else
    return 0;
}
