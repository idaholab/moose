//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowElementNormal.h"

registerMooseObject("PorousFlowApp", PorousFlowElementNormal);

InputParameters
PorousFlowElementNormal::validParams()
{
  InputParameters params = AuxKernel::validParams();
  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>("component", component, "The component to compute");

  params.addParam<RealVectorValue>("3D_default",
                                   RealVectorValue(0, 0, 1),
                                   "The value that will be produced for 3D elements, since such "
                                   "elements do not have a 'normal direction'");
  params.addParam<RealVectorValue>(
      "1D_perp",
      RealVectorValue(0, 0, 1),
      "The normal for all 1D elements will be perpendicular to this vector");

  params.addClassDescription(
      "AuxKernel to compute components of the element normal.  This is mostly designed for 2D "
      "elements living in 3D space, however, the 1D-element and 3D-element cases are handled as "
      "special cases.  The Variable for this AuxKernel must be an elemental Variable");

  return params;
}

PorousFlowElementNormal::PorousFlowElementNormal(const InputParameters & parameters)
  : AuxKernel(parameters),
    _component(getParam<MooseEnum>("component")),
    _1D_perp(getParam<RealVectorValue>("1D_perp")),
    _3D_default(getParam<RealVectorValue>("3D_default"))
{
  if (isNodal())
    paramError("variable", "The variable must be an elemental variable");
  if (_1D_perp.norm() == 0.0)
    paramError("1D_perp", "Must not be the zero vector");
  if (_3D_default.norm() == 0.0)
    paramError("3D_default", "Must not be the zero vector");
}

Real
PorousFlowElementNormal::computeValue()
{
  RealVectorValue n;
  const auto num_nodes = _current_elem->n_nodes();
  switch (_current_elem->dim())
  {
    case 1:
    {
      for (unsigned i = 0; i < num_nodes - 1; ++i)
      {
        RealVectorValue v = _current_elem->point((i + 1) % num_nodes) - _current_elem->point(i);
        n += v.cross(_1D_perp);
      }
      break;
    }
    case 2:
    {
      for (unsigned i = 0; i < num_nodes - 2; ++i)
      {
        RealVectorValue v1 = _current_elem->point((i + 1) % num_nodes) - _current_elem->point(i);
        RealVectorValue v2 =
            _current_elem->point((i + 2) % num_nodes) - _current_elem->point((i + 1) % num_nodes);
        n += v1.cross(v2);
      }
      break;
    }
    default:
      n = _3D_default;
  }
  return n.unit()(_component);
}
