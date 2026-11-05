//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetContactorAux.h"
#include "LevelSetContactor.h"

registerMooseObject("ContactApp", LevelSetContactorAux);

InputParameters
LevelSetContactorAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Writes a scalar component of a LevelSetContactor (signed distance "
                             "or a normal component) into an aux variable at the current node or "
                             "quadrature point.");
  params.addRequiredParam<UserObjectName>("contactor", "The LevelSetContactor to sample.");
  MooseEnum quantity("signed_distance normal_x normal_y normal_z");
  params.addRequiredParam<MooseEnum>("quantity", quantity, "Which scalar to write.");
  return params;
}

LevelSetContactorAux::LevelSetContactorAux(const InputParameters & p)
  : AuxKernel(p),
    _contactor(getUserObject<LevelSetContactor>("contactor")),
    _quantity(getParam<MooseEnum>("quantity"))
{
}

Real
LevelSetContactorAux::computeValue()
{
  const Point p = isNodal() ? static_cast<Point>(*_current_node) : _q_point[_qp];

  if (_quantity == "signed_distance")
    return _contactor.signedDistance(p);

  const RealVectorValue n = _contactor.normal(p);
  if (_quantity == "normal_x")
    return n(0);
  if (_quantity == "normal_y")
    return n(1);
  return n(2);
}
