//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMQuadratureFunctionCoefficientBase.h"
#include "MooseError.h"

#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"

namespace
{

bool
samePoint(const mfem::IntegrationPoint & a, const mfem::IntegrationPoint & b)
{
  constexpr mfem::real_t tol = 1e-12;
  auto close = [](mfem::real_t x, mfem::real_t y) { return !(std::abs(x - y) > tol); };
  return close(a.x, b.x) && close(a.y, b.y) && close(a.z, b.z) && close(a.weight, b.weight);
}
}

void
MFEMQuadratureFunctionCoefficientBase::checkIntegrationRule(const mfem::QuadratureFunction & qf,
                                                            mfem::ElementTransformation & T,
                                                            const mfem::IntegrationPoint & ip) const
{
  const mfem::QuadratureSpaceBase & qspace = *qf.GetSpace();
  const int el_idx = qspace.GetEntityIndex(T);
  // Entity not in this space (e.g. a boundary evaluation); mfem's Eval returns zero, nothing to
  // check.
  if (el_idx < 0)
    return;

  const mfem::IntegrationRule & stored_rule = qspace.GetIntRule(el_idx);
  // Fast path: the consuming integrator's point at this index coincides with the stored rule's,
  // so the rules match.
  if (ip.index < stored_rule.Size() && samePoint(stored_rule.IntPoint(ip.index), ip))
    return;

  // Rules differ. Recover the quadrature order the coefficient should have used by finding the
  // lowest order whose rule for this geometry reproduces the consuming integrator's point at
  // ip.index. This runs only on the error path.
  const int stored_order = qspace.GetOrder();
  const mfem::Geometry::Type geom = qspace.GetGeometry(el_idx);
  int suggested_order = -1;
  for (int order = 0; order <= 2 * stored_order + 64; ++order)
  {
    const mfem::IntegrationRule & candidate = mfem::IntRules.Get(geom, order);
    if (ip.index < candidate.Size() && samePoint(candidate.IntPoint(ip.index), ip))
    {
      suggested_order = order;
      break;
    }
  }

  if (suggested_order >= 0)
    mooseError("MFEM quadrature function '",
               _name,
               "' stores values on the order-",
               stored_order,
               " quadrature rule (",
               stored_rule.Size(),
               " points on this element), but it is being evaluated by an integrator using a "
               "different quadrature rule (",
               mfem::IntRules.Get(geom, suggested_order).Size(),
               " points). The stored values are indexed by quadrature point, so the orders must "
               "match. Set 'order = ",
               suggested_order,
               "' on '",
               _name,
               "'.");
  else
    mooseError("MFEM quadrature function '",
               _name,
               "' stores values on the order-",
               stored_order,
               " quadrature rule, but it is being evaluated by an integrator using a different "
               "quadrature rule. The stored values are indexed by quadrature point, so the orders "
               "must match; adjust 'order' on '",
               _name,
               "' to match the consuming integrator.");
}

#endif
