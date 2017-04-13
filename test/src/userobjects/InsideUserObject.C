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

#include "InsideUserObject.h"

template <>
InputParameters
validParams<InsideUserObject>()
{
  InputParameters params = validParams<InternalSideUserObject>();
  params.addParam<MaterialPropertyName>(
      "diffusivity",
      0.0,
      "The name of the diffusivity material property that will be used in the flux computation.");
  params.addParam<bool>(
      "use_old_prop",
      false,
      "A Boolean to indicate whether the current or old value of a material prop should be used.");
  params.addRequiredCoupledVar("variable", "the variable name");

  return params;
}

InsideUserObject::InsideUserObject(const InputParameters & parameters)
  : InternalSideUserObject(parameters),
    _u(coupledValue("variable")),
    _u_neighbor(coupledNeighborValue("variable")),
    _value(0.),
    _diffusivity_prop(getParam<bool>("use_old_prop") ? getMaterialPropertyOld<Real>("diffusivity")
                                                     : getMaterialProperty<Real>("diffusivity")),
    _neighbor_diffusivity_prop(getParam<bool>("use_old_prop")
                                   ? getNeighborMaterialPropertyOld<Real>("diffusivity")
                                   : getNeighborMaterialProperty<Real>("diffusivity"))
{
}

InsideUserObject::~InsideUserObject() {}

void
InsideUserObject::initialize()
{
  _value = 0;
}

void
InsideUserObject::execute()
{
  for (unsigned int qp = 0; qp < _q_point.size(); ++qp)
    _value += std::pow(_u[qp] - _u_neighbor[qp], 2) +
              (_diffusivity_prop[qp] + _neighbor_diffusivity_prop[qp]) / 2;
}

void
InsideUserObject::finalize()
{
  gatherSum(_value);
  _value = std::sqrt(_value);
}

void
InsideUserObject::threadJoin(const UserObject & uo)
{
  const InsideUserObject & u = dynamic_cast<const InsideUserObject &>(uo);
  _value += u._value;
}
