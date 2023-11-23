//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesMethods.h"
#include "MooseError.h"
#include "libmesh/vector_value.h"

namespace NS
{
int
delta(unsigned int i, unsigned int j)
{
  if (i == j)
    return 1;
  else
    return 0;
}

int
computeSign(const Real & a)
{
  return a > 0 ? 1 : (a < 0 ? -1 : 0);
}

unsigned int
getIndex(const Real & p, const std::vector<Real> & bounds)
{
  if (p < bounds.front() || p > bounds.back())
    mooseError("Point exceeds bounds of domain!");

  for (unsigned int i = 1; i < bounds.size(); ++i)
    if (p <= bounds[i])
      return i - 1;

  return bounds.size() - 2;
}

Real
reynoldsPropertyDerivative(
    const Real & Re, const Real & rho, const Real & mu, const Real & drho, const Real & dmu)
{
  return Re * (drho / std::max(rho, 1e-6) - dmu / std::max(mu, 1e-8));
}

Real
prandtlPropertyDerivative(const Real & mu,
                          const Real & cp,
                          const Real & k,
                          const Real & dmu,
                          const Real & dcp,
                          const Real & dk)
{
  return (k * (mu * dcp + cp * dmu) - mu * cp * dk) / std::max(k * k, 1e-8);
}

ADReal
findUStar(const ADReal & mu, const ADReal & rho, const ADReal & u, const Real dist)
{
  // usually takes about 3-4 iterations
  constexpr int MAX_ITERS{50};
  constexpr Real REL_TOLERANCE{1e-6};

  constexpr Real von_karman{0.4187};

  const ADReal nu = mu / rho;

  ADReal u_star = std::sqrt(nu * u / dist);

  // Newton-Raphson method to solve for u_star (friction velocity).
  for (int i = 0; i < MAX_ITERS; ++i)
  {
    ADReal residual = u_star / von_karman * std::log(u_star * dist / (0.111 * nu)) - u;
    ADReal deriv = (1 + std::log(u_star * dist / (0.111 * nu))) / von_karman;
    ADReal new_u_star = std::max(1e-20, u_star - residual / deriv);

    Real rel_err = std::abs((new_u_star.value() - u_star.value()) / new_u_star.value());

    u_star = new_u_star;
    if (rel_err < REL_TOLERANCE)
      return u_star;
  }

  mooseException("Could not find the wall friction velocity (mu: ",
                 mu,
                 " rho: ",
                 rho,
                 " velocity: ",
                 u,
                 " wall distance: ",
                 dist,
                 ")");
}

ADReal
findyPlus(const ADReal & mu, const ADReal & rho, const ADReal & u, const Real dist)
{
  // Fixed point iteration method to find y_plus
  // It should take 3 or 4 iterations
  constexpr int MAX_ITERS{10};
  constexpr Real REL_TOLERANCE{1e-2};

  constexpr Real von_karman{0.4187};
  constexpr Real E{9.793};

  const ADReal nu = mu / rho;
  ADReal yPlusLam = dist * u / nu;

  ADReal yPlusLast = 0.0;
  ADReal yPlus = yPlusLam;
  ADReal rev_yPlusLam = 1.0 / yPlusLam;

  ADReal kappa_time_Re = von_karman * u * dist / nu;
  unsigned int iters = 0;

  do
  {
    yPlusLast = yPlus;
    yPlus = (kappa_time_Re + yPlus) / (1.0 + std::log(E * yPlus));
  } while (rev_yPlusLam * (yPlus - yPlusLast) > REL_TOLERANCE && ++iters < MAX_ITERS);

  return std::max(0.0, yPlus);
}

ADReal
computeSpeed(const ADRealVectorValue & velocity)
{
  // if the velocity is zero, then the norm function call fails because AD tries to calculate the
  // derivatives which causes a divide by zero - because d/dx(sqrt(f(x))) = 1/2/sqrt(f(x))*df/dx.
  // So add a bit of noise (based on hitchhiker's guide to the galaxy's meaning of life number) to
  // avoid this failure mode.
  return isZero(velocity) ? 1e-42 : velocity.norm();
}

/// Bounded element maps for wall treatement
std::map<const Elem *, bool>
getWallBoundedElements(const std::vector<BoundaryName> & _wall_boundary_name,
                       const FEProblemBase & _fe_problem,
                       const SubProblem & _subproblem)
{

  // If the map has already been populated, we return it
  // We don't allow differential wall treatement for different models
  // This can induce physical errors
  // So, only 1 map exists for determining the wall bounded elements
  // if (_wall_bounded.size() > 0)
  //   return &_wall_bounded;

  std::map<const Elem *, bool> _wall_bounded;

  for (const auto & elem : _fe_problem.mesh().getMesh().element_ptr_range())
  {
    auto wall_bounded = false;
    for (unsigned int i_side = 0; i_side < elem->n_sides(); ++i_side)
    {
      const std::vector<BoundaryID> side_bnds = _subproblem.mesh().getBoundaryIDs(elem, i_side);
      for (const BoundaryName & name : _wall_boundary_name)
      {
        BoundaryID wall_id = _subproblem.mesh().getBoundaryID(name);
        for (BoundaryID side_id : side_bnds)
        {
          if (side_id == wall_id)
            wall_bounded = true;
        }
      }
    }
    _wall_bounded[elem] = wall_bounded;
  }
  return _wall_bounded;
}

/// Bounded element face distances for wall treatement
std::map<const Elem *, std::vector<Real>>
getWallDistance(const std::vector<BoundaryName> & _wall_boundary_name,
                const FEProblemBase & _fe_problem,
                const SubProblem & _subproblem)
{

  // If the map has already been populated, we return it
  // We don't allow differential wall treatement for different models
  // This can induce physical errors
  // So, only 1 map exists for determining the wall distances
  // if (_dist.size() > 0)
  //   return &_dist;

  std::map<const Elem *, std::vector<Real>> _dist;

  for (const auto & elem : _fe_problem.mesh().getMesh().element_ptr_range())
  {
    for (unsigned int i_side = 0; i_side < elem->n_sides(); ++i_side)
    {
      const std::vector<BoundaryID> side_bnds = _subproblem.mesh().getBoundaryIDs(elem, i_side);
      for (const BoundaryName & name : _wall_boundary_name)
      {
        BoundaryID wall_id = _subproblem.mesh().getBoundaryID(name);
        for (BoundaryID side_id : side_bnds)
        {
          if (side_id == wall_id)
          {
            const FaceInfo * const fi = _subproblem.mesh().faceInfo(elem, i_side);
            Real dist = std::abs((fi->elemCentroid() - fi->faceCentroid()) * fi->normal());
            _dist[elem].push_back(dist);
          }
        }
      }
    }
  }
  return _dist;
}

/// Bounded element face normals for wall treatement
std::map<const Elem *, std::vector<Point>>
getElementFaceNormal(const std::vector<BoundaryName> & _wall_boundary_name,
                     const FEProblemBase & _fe_problem,
                     const SubProblem & _subproblem)
{

  // If the map has already been populated, we return it
  // We don't allow differential wall treatement for different models
  // This can induce physical errors
  // So, only 1 map exists for determining the wall bounded elements' normals
  // if (_normal.size() > 0)
  //   return &_normal;

  std::map<const Elem *, std::vector<Point>> _normal;

  for (const auto & elem : _fe_problem.mesh().getMesh().element_ptr_range())
  {
    for (unsigned int i_side = 0; i_side < elem->n_sides(); ++i_side)
    {
      const std::vector<BoundaryID> side_bnds = _subproblem.mesh().getBoundaryIDs(elem, i_side);
      for (const BoundaryName & name : _wall_boundary_name)
      {
        BoundaryID wall_id = _subproblem.mesh().getBoundaryID(name);
        for (BoundaryID side_id : side_bnds)
        {
          if (side_id == wall_id)
          {
            const FaceInfo * const fi = _subproblem.mesh().faceInfo(elem, i_side);
            _normal[elem].push_back(fi->normal());
          }
        }
      }
    }
  }
  return _normal;
}

/// Face arguments to wall-bounded faces for wall tretement
std::map<const Elem *, std::vector<const FaceInfo *>>
getElementFaceArgs(const std::vector<BoundaryName> & _wall_boundary_name,
                   const FEProblemBase & _fe_problem,
                   const SubProblem & _subproblem)
{

  // If the map has already been populated, we return it
  // We don't allow differential wall treatement for different models
  // This can induce physical errors
  // So, only 1 map exists for determining the wall bounded elements' faces
  // if (_face_infos.size() > 0)
  //   return &_face_infos;

  std::map<const Elem *, std::vector<const FaceInfo *>> _face_infos;

  for (const auto & elem : _fe_problem.mesh().getMesh().element_ptr_range())
  {
    for (unsigned int i_side = 0; i_side < elem->n_sides(); ++i_side)
    {
      const std::vector<BoundaryID> side_bnds = _subproblem.mesh().getBoundaryIDs(elem, i_side);
      for (const BoundaryName & name : _wall_boundary_name)
      {
        BoundaryID wall_id = _subproblem.mesh().getBoundaryID(name);
        for (BoundaryID side_id : side_bnds)
        {
          if (side_id == wall_id)
          {
            const FaceInfo * fi = _subproblem.mesh().faceInfo(elem, i_side);
            _face_infos[elem].push_back(fi);
          }
        }
      }
    }
  }
  return _face_infos;
}
}
