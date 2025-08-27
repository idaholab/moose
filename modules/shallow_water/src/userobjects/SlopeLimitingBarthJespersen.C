//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SlopeLimitingBarthJespersen.h"
#include "libmesh/elem.h"

registerMooseObject("ShallowWaterApp", SlopeLimitingBarthJespersen);

InputParameters
SlopeLimitingBarthJespersen::validParams()
{
  InputParameters params = SlopeLimitingMultiDBase::validParams();
  params.addClassDescription("Barth-Jespersen limiter for multi-D MUSCL reconstruction.");
  return params;
}

SlopeLimitingBarthJespersen::SlopeLimitingBarthJespersen(const InputParameters & parameters)
  : SlopeLimitingMultiDBase(parameters)
{
}

std::vector<libMesh::RealGradient>
SlopeLimitingBarthJespersen::limitElementSlope() const
{
  const dof_id_type id = _current_elem->id();
  const auto & grad_rs = _rslope.getElementSlope(id);
  const auto & u0 = _rslope.getElementAverageValue(id);
  const Point xc = _current_elem->vertex_average();

  std::vector<libMesh::RealGradient> grad_limited = grad_rs;
  if (grad_rs.empty())
    return grad_limited;

  // number of variables
  const unsigned int nvars = grad_rs.size();

  // Loop sides to compute limiter factors
  for (unsigned int k = 0; k < nvars; ++k)
  {
    Real phi = 1.0;
    for (unsigned int s = 0; s < _current_elem->n_sides(); ++s)
    {
      const Elem * neigh = _current_elem->neighbor_ptr(s);
      const Point xs =
          neigh ? _rslope.getSideCentroid(id, neigh->id()) : _rslope.getBoundarySideCentroid(id, s);
      const Real uface = u0[k] + grad_rs[k] * (xs - xc);
      Real umin = u0[k];
      Real umax = u0[k];
      if (neigh)
      {
        const auto & un = _rslope.getElementAverageValue(neigh->id());
        umin = std::min(u0[k], un[k]);
        umax = std::max(u0[k], un[k]);
      }
      // Update limiter
      const Real du = uface - u0[k];
      if (du > 1e-16)
        phi = std::min(phi, (umax - u0[k]) / du);
      else if (du < -1e-16)
        phi = std::min(phi, (umin - u0[k]) / du);
    }
    phi = std::max(0.0, std::min(1.0, phi));
    grad_limited[k] = phi * grad_rs[k];
  }

  return grad_limited;
}
