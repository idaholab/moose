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
