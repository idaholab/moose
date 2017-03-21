/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVTimeStepLimit.h"
#include "MooseMesh.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<CNSFVTimeStepLimit>()
{
  InputParameters params = validParams<ElementPostprocessor>();
  params.addClassDescription(
      "A PostProcessor object to calculate the allowable time step size for the CNS equations.");
  params.addParam<Real>("cfl", 0.8, "CFL number");
  return params;
}

CNSFVTimeStepLimit::CNSFVTimeStepLimit(const InputParameters & parameters)
  : ElementPostprocessor(parameters),
    _dim(_mesh.dimension()),
    _cfl(getParam<Real>("cfl")),
    _vmag(getMaterialProperty<Real>("velocity_magnitude")),
    _csou(getMaterialProperty<Real>("speed_of_sound"))
{
}

CNSFVTimeStepLimit::~CNSFVTimeStepLimit() {}

void
CNSFVTimeStepLimit::initialize()
{
  _value = std::numeric_limits<Real>::max();
}

void
CNSFVTimeStepLimit::execute()
{
  Real h_min = _current_elem->hmin();
  for (unsigned qp = 0; qp < _qrule->n_points(); ++qp)
    _value = std::min(_value, _cfl * h_min / (_vmag[qp] + _csou[qp]));
}

Real
CNSFVTimeStepLimit::getValue()
{
  gatherMin(_value);
  return _value;
}

void
CNSFVTimeStepLimit::threadJoin(const UserObject & uo)
{
  const CNSFVTimeStepLimit & pps = dynamic_cast<const CNSFVTimeStepLimit &>(uo);
  _value = std::min(_value, pps._value);
}
