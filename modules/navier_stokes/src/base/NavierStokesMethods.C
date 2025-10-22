//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesMethods.h"
#include "MooseError.h"
#include "libmesh/vector_value.h"
#include "NS.h"

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

template <typename T>
T
findUStar(const T & mu, const T & rho, const T & u, const Real dist)
{
  // usually takes about 3-4 iterations
  constexpr int MAX_ITERS{50};
  constexpr Real REL_TOLERANCE{1e-6};

  // Check inputs
  mooseAssert(mu > 0, "Need a strictly positive viscosity");
  mooseAssert(rho > 0, "Need a strictly positive density");
  mooseAssert(u > 0, "Need a strictly positive velocity");
  mooseAssert(dist > 0, "Need a strictly positive wall distance");

  const T nu = mu / rho;

  // Wall-function linearized guess
  const Real a_c = 1 / NS::von_karman_constant;
  const T b_c = 1.0 / NS::von_karman_constant * (std::log(NS::E_turb_constant * dist / mu) + 1.0);
  const T & c_c = u;

  /// This is important to reduce the number of nonlinear iterations
  T u_star = std::max(1e-20, (-b_c + std::sqrt(std::pow(b_c, 2) + 4.0 * a_c * c_c)) / (2.0 * a_c));

  // Newton-Raphson method to solve for u_star (friction velocity).
  for (int i = 0; i < MAX_ITERS; ++i)
  {
    T residual =
        u_star / NS::von_karman_constant * std::log(NS::E_turb_constant * u_star * dist / nu) - u;
    T deriv = (1.0 + std::log(NS::E_turb_constant * u_star * dist / nu)) / NS::von_karman_constant;
    T new_u_star = std::max(1e-20, u_star - residual / deriv);

    Real rel_err =
        std::abs(MetaPhysicL::raw_value(new_u_star - u_star) / MetaPhysicL::raw_value(new_u_star));

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
template Real findUStar<Real>(const Real & mu, const Real & rho, const Real & u, const Real dist);
template ADReal
findUStar<ADReal>(const ADReal & mu, const ADReal & rho, const ADReal & u, const Real dist);

template <typename T>
T
findyPlus(const T & mu, const T & rho, const T & u, const Real dist)
{
  // Fixed point iteration method to find y_plus
  // It should take 3 or 4 iterations
  constexpr int MAX_ITERS{10};
  constexpr Real REL_TOLERANCE{1e-2};

  // Check inputs
  mooseAssert(mu > 0, "Need a strictly positive viscosity");
  mooseAssert(u > 0, "Need a strictly positive velocity");
  mooseAssert(rho > 0, "Need a strictly positive density");
  mooseAssert(dist > 0, "Need a strictly positive wall distance");

  Real yPlusLast = 0.0;
  T yPlus = dist * u * rho / mu; // Assign initial value to laminar
  const Real rev_yPlusLam = 1.0 / MetaPhysicL::raw_value(yPlus);
  const T kappa_time_Re = NS::von_karman_constant * u * dist / (mu / rho);
  unsigned int iters = 0;

  do
  {
    yPlusLast = MetaPhysicL::raw_value(yPlus);
    // Negative y plus does not make sense
    yPlus = std::max(NS::min_y_plus, yPlus);
    yPlus = (kappa_time_Re + yPlus) / (1.0 + std::log(NS::E_turb_constant * yPlus));
  } while (std::abs(rev_yPlusLam * (MetaPhysicL::raw_value(yPlus) - yPlusLast)) > REL_TOLERANCE &&
           ++iters < MAX_ITERS);

  return std::max(NS::min_y_plus, yPlus);
}
template Real findyPlus<Real>(const Real & mu, const Real & rho, const Real & u, Real dist);
template ADReal
findyPlus<ADReal>(const ADReal & mu, const ADReal & rho, const ADReal & u, Real dist);

template <typename T>
T
computeSpeed(const libMesh::VectorValue<T> & velocity)
{
  // if the velocity is zero, then the norm function call fails because AD tries to calculate the
  // derivatives which causes a divide by zero - because d/dx(sqrt(f(x))) = 1/2/sqrt(f(x))*df/dx.
  // So add a bit of noise (based on hitchhiker's guide to the galaxy's meaning of life number) to
  // avoid this failure mode.
  return isZero(velocity) ? 1e-42 : velocity.norm();
}
template Real computeSpeed<Real>(const libMesh::VectorValue<Real> & velocity);
template ADReal computeSpeed<ADReal>(const libMesh::VectorValue<ADReal> & velocity);

template <typename T>
T
computeShearStrainRateNormSquared(const Moose::Functor<T> & u,
                                  const Moose::Functor<T> * v,
                                  const Moose::Functor<T> * w,
                                  const Moose::ElemArg & elem_arg,
                                  const Moose::StateArg & state)
{
  const auto & grad_u = u.gradient(elem_arg, state);
  const T Sij_xx = 2.0 * grad_u(0);
  T Sij_xy = 0.0;
  T Sij_xz = 0.0;
  T Sij_yy = 0.0;
  T Sij_yz = 0.0;
  T Sij_zz = 0.0;

  const T grad_xx = grad_u(0);
  T grad_xy = 0.0;
  T grad_xz = 0.0;
  T grad_yx = 0.0;
  T grad_yy = 0.0;
  T grad_yz = 0.0;
  T grad_zx = 0.0;
  T grad_zy = 0.0;
  T grad_zz = 0.0;

  T trace = Sij_xx / 3.0;

  if (v) // dim >= 2
  {
    const auto & grad_v = (*v).gradient(elem_arg, state);
    Sij_xy = grad_u(1) + grad_v(0);
    Sij_yy = 2.0 * grad_v(1);

    grad_xy = grad_u(1);
    grad_yx = grad_v(0);
    grad_yy = grad_v(1);

    trace += Sij_yy / 3.0;

    if (w) // dim >= 3
    {
      const auto & grad_w = (*w).gradient(elem_arg, state);

      Sij_xz = grad_u(2) + grad_w(0);
      Sij_yz = grad_v(2) + grad_w(1);
      Sij_zz = 2.0 * grad_w(2);

      grad_xz = grad_u(2);
      grad_yz = grad_v(2);
      grad_zx = grad_w(0);
      grad_zy = grad_w(1);
      grad_zz = grad_w(2);

      trace += Sij_zz / 3.0;
    }
  }

  return (Sij_xx - trace) * grad_xx + Sij_xy * grad_xy + Sij_xz * grad_xz + Sij_xy * grad_yx +
         (Sij_yy - trace) * grad_yy + Sij_yz * grad_yz + Sij_xz * grad_zx + Sij_yz * grad_zy +
         (Sij_zz - trace) * grad_zz;
}
template Real computeShearStrainRateNormSquared<Real>(const Moose::Functor<Real> & u,
                                                      const Moose::Functor<Real> * v,
                                                      const Moose::Functor<Real> * w,
                                                      const Moose::ElemArg & elem_arg,
                                                      const Moose::StateArg & state);
template ADReal computeShearStrainRateNormSquared<ADReal>(const Moose::Functor<ADReal> & u,
                                                          const Moose::Functor<ADReal> * v,
                                                          const Moose::Functor<ADReal> * w,
                                                          const Moose::ElemArg & elem_arg,
                                                          const Moose::StateArg & state);

/// Bounded element maps for wall treatment
void
getWallBoundedElements(const std::vector<BoundaryName> & wall_boundary_names,
                       const FEProblemBase & fe_problem,
                       const SubProblem & subproblem,
                       const std::set<SubdomainID> & block_ids,
                       std::unordered_set<const Elem *> & wall_bounded)
{

  wall_bounded.clear();
  const auto wall_boundary_ids = subproblem.mesh().getBoundaryIDs(wall_boundary_names);

  // We define these lambdas so that we can fetch the bounded elements from other
  // processors.
  auto gather_functor = [&subproblem, &wall_bounded](const processor_id_type libmesh_dbg_var(pid),
                                                     const std::vector<dof_id_type> & elem_ids,
                                                     std::vector<unsigned char> & data_to_fill)
  {
    mooseAssert(pid != subproblem.processor_id(), "We shouldn't be gathering from ourselves.");
    data_to_fill.resize(elem_ids.size());

    const auto & mesh = subproblem.mesh().getMesh();

    for (const auto i : index_range(elem_ids))
    {
      const auto elem = mesh.elem_ptr(elem_ids[i]);
      data_to_fill[i] = wall_bounded.count(elem) != 0;
    }
  };

  auto action_functor = [&subproblem, &wall_bounded](const processor_id_type libmesh_dbg_var(pid),
                                                     const std::vector<dof_id_type> & elem_ids,
                                                     const std::vector<unsigned char> & filled_data)
  {
    mooseAssert(pid != subproblem.processor_id(),
                "The request filler shouldn't have been ourselves");
    mooseAssert(elem_ids.size() == filled_data.size(), "I think these should be the same size");

    const auto & mesh = subproblem.mesh().getMesh();

    for (const auto i : index_range(elem_ids))
    {
      const auto elem = mesh.elem_ptr(elem_ids[i]);
      if (filled_data[i])
        wall_bounded.insert(elem);
    }
  };

  // We need these elements from other processors
  std::unordered_map<processor_id_type, std::vector<dof_id_type>> elem_ids_requested;

  for (const auto & elem : fe_problem.mesh().getMesh().active_local_element_ptr_range())
    if (block_ids.find(elem->subdomain_id()) != block_ids.end())
      for (const auto i_side : elem->side_index_range())
      {
        // This is needed because in some cases the internal boundary is registered
        // to the neighbor element
        std::set<BoundaryID> combined_side_bds;
        const auto & side_bnds = subproblem.mesh().getBoundaryIDs(elem, i_side);
        combined_side_bds.insert(side_bnds.begin(), side_bnds.end());
        if (const auto neighbor = elem->neighbor_ptr(i_side))
        {
          const auto neighbor_side = neighbor->which_neighbor_am_i(elem);
          const auto & neighbor_bnds = subproblem.mesh().getBoundaryIDs(neighbor, neighbor_side);
          combined_side_bds.insert(neighbor_bnds.begin(), neighbor_bnds.end());

          // If the neighbor lives on the first layer of the ghost region then we would
          // like to grab its value as well (if it exists)
          if (neighbor->processor_id() != subproblem.processor_id() &&
              block_ids.find(neighbor->subdomain_id()) != block_ids.end())
            elem_ids_requested[neighbor->processor_id()].push_back(neighbor->id());
        }

        for (const auto wall_id : wall_boundary_ids)
          if (combined_side_bds.count(wall_id))
          {
            wall_bounded.insert(elem);
            break;
          }
      }

  unsigned char * bool_ex = nullptr;
  TIMPI::pull_parallel_vector_data(
      subproblem.comm(), elem_ids_requested, gather_functor, action_functor, bool_ex);
}

/// Bounded element face distances for wall treatment
void
getWallDistance(const std::vector<BoundaryName> & wall_boundary_name,
                const FEProblemBase & fe_problem,
                const SubProblem & subproblem,
                const std::set<SubdomainID> & block_ids,
                std::map<const Elem *, std::vector<Real>> & dist_map)
{
  dist_map.clear();
  const auto wall_boundary_ids = subproblem.mesh().getBoundaryIDs(wall_boundary_name);

  for (const auto & elem : fe_problem.mesh().getMesh().active_local_element_ptr_range())
    if (block_ids.find(elem->subdomain_id()) != block_ids.end())
      for (const auto i_side : elem->side_index_range())
      {
        // This is needed because in some cases the internal boundary is registered
        // to the neighbor element
        std::set<BoundaryID> combined_side_bds;
        const auto & side_bnds = subproblem.mesh().getBoundaryIDs(elem, i_side);
        combined_side_bds.insert(side_bnds.begin(), side_bnds.end());
        if (const auto neighbor = elem->neighbor_ptr(i_side))
        {
          const auto neighbor_side = neighbor->which_neighbor_am_i(elem);
          const std::vector<BoundaryID> & neighbor_bnds =
              subproblem.mesh().getBoundaryIDs(neighbor, neighbor_side);
          combined_side_bds.insert(neighbor_bnds.begin(), neighbor_bnds.end());
        }

        for (const auto wall_id : wall_boundary_ids)
          if (combined_side_bds.count(wall_id))
          {
            // The list below stores the face infos with respect to their owning elements,
            // depending on the block restriction we might encounter situations where the
            // element outside of the block owns the face info.
            const auto & neighbor = elem->neighbor_ptr(i_side);
            const auto elem_has_fi = Moose::FV::elemHasFaceInfo(*elem, neighbor);
            const auto & elem_for_fi = elem_has_fi ? elem : neighbor;
            const auto side = elem_has_fi ? i_side : neighbor->which_neighbor_am_i(elem);

            const FaceInfo * const fi = subproblem.mesh().faceInfo(elem_for_fi, side);
            const auto & elem_centroid = elem_has_fi ? fi->elemCentroid() : fi->neighborCentroid();
            const Real dist = std::abs((elem_centroid - fi->faceCentroid()) * fi->normal());
            dist_map[elem].push_back(dist);
          }
      }
}

/// Face arguments to wall-bounded faces for wall treatment
void
getElementFaceArgs(const std::vector<BoundaryName> & wall_boundary_name,
                   const FEProblemBase & fe_problem,
                   const SubProblem & subproblem,
                   const std::set<SubdomainID> & block_ids,
                   std::map<const Elem *, std::vector<const FaceInfo *>> & face_info_map)
{
  face_info_map.clear();
  const auto wall_boundary_ids = subproblem.mesh().getBoundaryIDs(wall_boundary_name);

  for (const auto & elem : fe_problem.mesh().getMesh().active_local_element_ptr_range())
    if (block_ids.find(elem->subdomain_id()) != block_ids.end())
      for (const auto i_side : elem->side_index_range())
      {
        // This is needed because in some cases the internal boundary is registered
        // to the neighbor element
        std::set<BoundaryID> combined_side_bds;
        const auto & side_bnds = subproblem.mesh().getBoundaryIDs(elem, i_side);
        combined_side_bds.insert(side_bnds.begin(), side_bnds.end());
        if (elem->neighbor_ptr(i_side) && !elem->neighbor_ptr(i_side)->is_remote())
        {
          const auto neighbor = elem->neighbor_ptr(i_side);
          const auto neighbor_side = neighbor->which_neighbor_am_i(elem);
          const std::vector<BoundaryID> & neighbor_bnds =
              subproblem.mesh().getBoundaryIDs(neighbor, neighbor_side);
          combined_side_bds.insert(neighbor_bnds.begin(), neighbor_bnds.end());
        }

        for (const auto wall_id : wall_boundary_ids)
          if (combined_side_bds.count(wall_id))
          {
            // The list below stores the face infos with respect to their owning elements,
            // depending on the block restriction we might encounter situations where the
            // element outside of the block owns the face info.
            const auto & neighbor = elem->neighbor_ptr(i_side);
            const auto elem_has_fi = Moose::FV::elemHasFaceInfo(*elem, neighbor);
            const auto & elem_for_fi = elem_has_fi ? elem : neighbor;
            const auto side = elem_has_fi ? i_side : neighbor->which_neighbor_am_i(elem);

            const FaceInfo * const fi = subproblem.mesh().faceInfo(elem_for_fi, side);
            face_info_map[elem].push_back(fi);
          }
      }
}
}
