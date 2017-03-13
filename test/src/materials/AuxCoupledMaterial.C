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

#include "AuxCoupledMaterial.h"

template<>
InputParameters validParams<AuxCoupledMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("variable", "The variable to be coupled.");
  return params;
}

AuxCoupledMaterial::AuxCoupledMaterial(const InputParameters & parameters) :
    Material(parameters),
    _variable(coupledNodalValue("variable")),
    _mat_prop(declareProperty<Real>("mat_prop")),
    _mat_prop_old(declarePropertyOld<Real>("mat_prop"))
{
}

void
AuxCoupledMaterial::initQpStatefulProperties()
{
  _mat_prop[_qp] = _variable[_qp];
}

void
AuxCoupledMaterial::computeQpProperties()
{
}
