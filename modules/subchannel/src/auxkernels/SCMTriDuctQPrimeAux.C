//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMTriDuctQPrimeAux.h"

registerMooseObject("SubChannelApp", SCMTriDuctQPrimeAux);
registerMooseObjectRenamed("SubChannelApp",
                           TriDuctQPrimeAux,
                           "06/30/2025 24:00",
                           SCMTriDuctQPrimeAux);

InputParameters
SCMTriDuctQPrimeAux::validParams()
{
  InputParameters params = DiffusionFluxAux::validParams();
  params.addClassDescription("Axial heat rate on duct surface");
  return params;
}

SCMTriDuctQPrimeAux::SCMTriDuctQPrimeAux(const InputParameters & parameters)
  : DiffusionFluxAux(parameters)
{
}

Real
SCMTriDuctQPrimeAux::computeValue()
{
  Real cell_width = 0;

  for (const auto s : make_range(_current_elem->n_sides()))
  {
    // External sides have the dimension we want to find
    if (!_current_elem->neighbor_ptr(s))
    {
      const auto side = _current_elem->side_ptr(s);

      // Exclude the side on the axial edge of the assembly
      const auto n0 = side->node_ref(0);
      const auto n1 = side->node_ref(1);
      const auto n2 = side->node_ref(2);
      const auto normal = Point(n1 - n0).cross(n2 - n1);
      if (std::abs(normal * _normals[_qp]) < TOLERANCE)
        continue;

      // Loop on edges on the side, to find the one that is "radial"
      for (const auto e : side->edge_index_range())
      {
        const auto ne0 = side->node_ref(side->local_edge_node(e, 0));
        const auto ne1 = side->node_ref(side->local_edge_node(e, 1));
        if (std::abs(Point(ne0 - ne1) * Point(0, 0, 1)) < TOLERANCE)
        {
          cell_width = (ne0 - ne1).norm();
          break;
        }
      }
    }
  }
  if (!cell_width)
    mooseError("Cell width detection failed at element ", *_current_elem);
  return DiffusionFluxAux::computeValue() * cell_width;
}
