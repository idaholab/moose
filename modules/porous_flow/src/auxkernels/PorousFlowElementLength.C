//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowElementLength.h"

registerMooseObject("PorousFlowApp", PorousFlowElementLength);

InputParameters
PorousFlowElementLength::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addCoupledVar("direction",
                       "Direction (3-component vector) along which to compute the length.  This "
                       "may be 3 real numbers, or 3 variables.");

  params.addClassDescription(
      "AuxKernel to compute the 'length' of elements along a given direction.  A plane is "
      "constructed through the element's centroid, with normal equal to the direction given.  The "
      "average of the distance of the nodal positions to this plane is the 'length' returned.  The "
      "Variable for this AuxKernel must be an elemental Variable");

  return params;
}

PorousFlowElementLength::PorousFlowElementLength(const InputParameters & parameters)
  : AuxKernel(parameters),
    _num_direction(coupledComponents("direction")),
    _direction_x(coupledValue("direction", 0)),
    // if _num_direction!=3,an informative error message is produced below
    _direction_y(coupledValue("direction", std::min(_num_direction - 1, (unsigned)1))),
    _direction_z(coupledValue("direction", std::min(_num_direction - 1, (unsigned)2)))
{
  if (isNodal())
    paramError("variable", "The variable must be an elemental variable");
  if (_num_direction != 3)
    paramError("direction", "Three values or variables must be provided");
}

Real
PorousFlowElementLength::computeValue()
{
  const auto direction =
      RealVectorValue(_direction_x[_qp], _direction_y[_qp], _direction_z[_qp]).unit();
  const auto centroid = _current_elem->vertex_average();
  Real length = 0.0;
  for (const auto & node : _current_elem->node_ref_range())
    length += std::abs((node - centroid) * direction);
  length /= _current_elem->n_nodes();
  return length;
}
