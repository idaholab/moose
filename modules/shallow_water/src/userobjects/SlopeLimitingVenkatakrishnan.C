//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SlopeLimitingVenkatakrishnan.h"

#include "libmesh/elem.h"

registerMooseObject("ShallowWaterApp", SlopeLimitingVenkatakrishnan);

InputParameters
SlopeLimitingVenkatakrishnan::validParams()
{
  InputParameters params = SlopeLimitingMultiDBase::validParams();
  params.addClassDescription(
      "Venkatakrishnan limiter for multi-D MUSCL reconstruction with smooth beta tuning.");
  params.addParam<Real>("beta", 1.0, "Smoothing parameter for Venkatakrishnan limiter");
  params.addParam<bool>("couple_momentum_to_h",
                        true,
                        "Apply depth limiter factor to momentum components as well");
  return params;
}

SlopeLimitingVenkatakrishnan::SlopeLimitingVenkatakrishnan(const InputParameters & parameters)
  : SlopeLimitingMultiDBase(parameters),
    _beta(getParam<Real>("beta")),
    _couple_momentum_to_h(getParam<bool>("couple_momentum_to_h"))
{
}

static inline Real phi_venkat(Real r, Real beta)
{
  // Smooth, differentiable limiter in [0,1], monotone in r >= 0
  // Typical form: (r^2 + 2 r + beta)/(r^2 + r + 2 + beta)
  if (r <= 0.0)
    return 0.0;
  const Real r2 = r * r;
  const Real num = r2 + 2.0 * r + beta;
  const Real den = r2 + r + 2.0 + beta;
  return std::min(1.0, std::max(0.0, num / den));
}

std::vector<libMesh::RealGradient>
SlopeLimitingVenkatakrishnan::limitElementSlope() const
{
  const dof_id_type id = _current_elem->id();
  const auto & grad_rs = _rslope.getElementSlope(id);
  const auto & u0 = _rslope.getElementAverageValue(id);
  const Point xc = _current_elem->vertex_average();

  std::vector<libMesh::RealGradient> grad_limited = grad_rs;
  if (grad_rs.empty())
    return grad_limited;

  const unsigned int nvars = grad_rs.size();
  std::vector<Real> phi_var(nvars, 1.0);

  for (unsigned int k = 0; k < nvars; ++k)
  {
    Real phi_k = 1.0;
    for (unsigned int s = 0; s < _current_elem->n_sides(); ++s)
    {
      const Elem * neigh = _current_elem->neighbor_ptr(s);
      const Point xs = neigh ? _rslope.getSideCentroid(id, neigh->id())
                             : _rslope.getBoundarySideCentroid(id, s);
      const Real uface = u0[k] + grad_rs[k] * (xs - xc);
      Real umin = u0[k];
      Real umax = u0[k];
      if (neigh)
      {
        const auto & un = _rslope.getElementAverageValue(neigh->id());
        umin = std::min(u0[k], un[k]);
        umax = std::max(u0[k], un[k]);
      }
      const Real du = uface - u0[k];
      if (du > 1e-16)
      {
        const Real r = (umax - u0[k]) / du;
        phi_k = std::min(phi_k, phi_venkat(r, _beta));
      }
      else if (du < -1e-16)
      {
        const Real r = (u0[k] - umin) / (-du);
        phi_k = std::min(phi_k, phi_venkat(r, _beta));
      }
      // if du ~ 0, no constraint from this side
    }
    phi_var[k] = std::max(0.0, std::min(1.0, phi_k));
  }

  // Optionally couple momentum to depth limiter for robustness
  if (_couple_momentum_to_h && nvars >= 3)
  {
    const Real phi_h = phi_var[0];
    phi_var[1] = std::min(phi_var[1], phi_h);
    phi_var[2] = std::min(phi_var[2], phi_h);
  }

  for (unsigned int k = 0; k < nvars; ++k)
    grad_limited[k] = phi_var[k] * grad_rs[k];

  return grad_limited;
}

