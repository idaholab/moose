//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeMortarFunctor.h"
#include "FEProblemBase.h"
#include "SubProblem.h"
#include "Assembly.h"
#include "ADMortarConstraint.h"
#include "AutomaticMortarGeneration.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "SwapBackSentinel.h"
#include "MooseLagrangeHelpers.h"

#include "libmesh/fe_base.h"
#include "libmesh/quadrature.h"
#include "libmesh/elem.h"
#include "libmesh/point.h"
#include "libmesh/mesh_base.h"
#include "libmesh/enum_to_string.h"

#include "metaphysicl/dualnumberarray.h"

#include "Eigen/Dense"

typedef DualNumber<Real, NumberArray<2, Real>> Dual2;

ComputeMortarFunctor::ComputeMortarFunctor(
    const std::vector<std::shared_ptr<MortarConstraintBase>> & mortar_constraints,
    const AutomaticMortarGeneration & amg,
    SubProblem & subproblem,
    FEProblemBase & fe_problem,
    bool displaced)
  : _amg(amg),
    _subproblem(subproblem),
    _fe_problem(fe_problem),
    _displaced(displaced),
    _assembly(_subproblem.assembly(0)),
    _qrule_msm(_assembly.qRuleMortar()),
    _JxW_msm(_assembly.jxWMortar())
{
  // Construct the mortar constraints we will later loop over
  for (auto mc : mortar_constraints)
    _mortar_constraints.push_back(mc.get());

  const auto & primary_secondary_boundary_id_pairs = _amg.primarySecondaryBoundaryIDPairs();
  mooseAssert(primary_secondary_boundary_id_pairs.size() == 1,
              "We currently only support a single primary-secondary ID pair per mortar segment "
              "mesh. We can probably support this without too much trouble if you want to contact "
              "a MOOSE developer.");
  _primary_boundary_id = primary_secondary_boundary_id_pairs[0].first;
  _secondary_boundary_id = primary_secondary_boundary_id_pairs[0].second;
}

void
ComputeMortarFunctor::operator()()
{
  // Set required material properties
  std::set<unsigned int> needed_mat_props;
  for (const auto & mc : _mortar_constraints)
  {
    const auto & mp_deps = mc->getMatPropDependencies();
    needed_mat_props.insert(mp_deps.begin(), mp_deps.end());
  }
  _fe_problem.setActiveMaterialProperties(needed_mat_props, /*tid=*/0);

  // For 3D mortar get index for retrieving sub-element info
  unsigned int secondary_sub_elem_ind = 0, primary_sub_elem_ind = 0;
  if (_amg.dim() == 3)
  {
    secondary_sub_elem_ind = _amg.mortarSegmentMesh().get_elem_integer_index("secondary_sub_elem");
    primary_sub_elem_ind = _amg.mortarSegmentMesh().get_elem_integer_index("primary_sub_elem");
  }

  unsigned int num_cached = 0;

  // Compile map of secondary to msm elements
  // All this could be computed in AutomaticMortarGeneration, will suffice for now but
  // in the future may want to move for optimization, especially on non-displaced meshes
  std::map<const Elem *, std::vector<Elem *>> secondary_elems_to_mortar_segments;

  for (const auto msm_elem : _amg.mortarSegmentMesh().active_local_element_ptr_range())
  {
    const MortarSegmentInfo & msinfo =
        libmesh_map_find(_amg.mortarSegmentMeshElemToInfo(), msm_elem);
    const Elem * secondary_face_elem = msinfo.secondary_elem;

    // Compute fraction of volume of secondary element the segment accounts for
    const Real volume_fraction = msm_elem->volume() / secondary_face_elem->volume();

    // Neglect small segments to avoid negative Jacobian errors for 0 volume segments
    if (volume_fraction < TOLERANCE)
      continue;

    secondary_elems_to_mortar_segments[secondary_face_elem].push_back(msm_elem);
  }

  // Loop through secondary elements, accumulating quadrature points for all corresponding mortar
  // segments
  for (const auto & elem_to_msm : secondary_elems_to_mortar_segments)
  {
    const Elem * secondary_face_elem = elem_to_msm.first;
    const std::vector<Elem *> & msm_elems = elem_to_msm.second;

    // Need to be able to check if there's edge dropping, in 3D we can't just compare

    // Map mortar segment integration points to primary and secondary sides
    // Note points for segments will be held contiguously to allow reinit without moving
    // cleaner way to do this would be with contiguously allocated 2D array but would either
    // need to move into vector for calling
    std::vector<Point> secondary_xi_pts, primary_xi_pts;

    std::vector<Real> JxW;

#ifndef NDEBUG
    unsigned int expected_length = 0;
#endif


    // Loop through contributing msm elements
    for (const auto msm_elem : msm_elems)
    {
      // Initialize mortar segment quadrature and compute JxW
      _subproblem.reinitMortarElem(msm_elem);

      // Get a reference to the MortarSegmentInfo for this Elem.
      const MortarSegmentInfo & msinfo = _amg.mortarSegmentMeshElemToInfo().at(msm_elem);

      if (msm_elem->dim() == 1)
      {
        for (unsigned int qp = 0; qp < _qrule_msm->n_points(); qp++)
        {
          const Real eta = _qrule_msm->qp(qp)(0);

          // Map quadrature points to secondary side
          const Real xi1_eta = 0.5 * (1 - eta) * msinfo.xi1_a + 0.5 * (1 + eta) * msinfo.xi1_b;
          secondary_xi_pts.push_back(xi1_eta);

          // Map quadrature points to primary side
          const Real xi2_eta = 0.5 * (1 - eta) * msinfo.xi2_a + 0.5 * (1 + eta) * msinfo.xi2_b;
          primary_xi_pts.push_back(xi2_eta);
        }
      }
      else
      {
        // Now that has_primary logic gone, could combine these to make more efficient
        projectQPoints3d(msm_elem, msinfo.secondary_elem, secondary_sub_elem_ind, secondary_xi_pts);
        projectQPoints3d(msm_elem, msinfo.primary_elem, primary_sub_elem_ind, primary_xi_pts);
      }

      // If edge dropping case we need JxW on the msm to compute dual shape functions
      if (_assembly.needDual())
        std::copy(std::begin(_JxW_msm), std::end(_JxW_msm), std::back_inserter(JxW));

#ifndef NDEBUG
      // Verify that the expected number of quadrature points have been inserted
      expected_length += _qrule_msm->n_points();
      mooseAssert(secondary_xi_pts.size() == expected_length,
                  "Fewer than expected secondary quadrature points");
      mooseAssert(primary_xi_pts.size() == expected_length,
                  "Fewer than expected primary quadrature points");

      if (_assembly.needDual())
        mooseAssert(JxW.size() == expected_length, "Fewer than expected JxW values computed");
#endif
    }

    // Reinit dual shape coeffs if dual shape functions needed
    if (_assembly.needDual())
      _assembly.reinitDual(secondary_face_elem, secondary_xi_pts, JxW);

    unsigned int n_segment = 0;

    // Loop through contributing msm elements, computing residual and Jacobian this time
    for (const auto msm_elem : msm_elems)
    {
      n_segment++;

      // These will hold quadrature points for each segment
      std::vector<Point> xi1_pts, xi2_pts;

      // Get a reference to the MortarSegmentInfo for this Elem.
      const MortarSegmentInfo & msinfo = _amg.mortarSegmentMeshElemToInfo().at(msm_elem);

      // Set the secondary interior parent and side ids
      const Elem * secondary_ip = msinfo.secondary_elem->interior_parent();
      unsigned int secondary_side_id = secondary_ip->which_side_am_i(msinfo.secondary_elem);

      // Set the primary interior parent and side ids
      const Elem * primary_ip = msinfo.primary_elem->interior_parent();
      unsigned int primary_side_id = primary_ip->which_side_am_i(msinfo.primary_elem);

      // Compute a JxW for the actual mortar segment element (not the lower dimensional element on
      // the secondary face!)
      _subproblem.reinitMortarElem(msm_elem);

      // Extract previously computed mapped quadrature points for secondary and primary face
      // elements
      const unsigned int start = (n_segment - 1) * _qrule_msm->n_points();
      const unsigned int end = n_segment * _qrule_msm->n_points();
      xi1_pts.insert(
          xi1_pts.begin(), secondary_xi_pts.begin() + start, secondary_xi_pts.begin() + end);
      xi2_pts.insert(xi2_pts.begin(), primary_xi_pts.begin() + start, primary_xi_pts.begin() + end);

      // Compute secondary element normals (for computing residual)
      const auto normals = _amg.getNormals(*msinfo.secondary_elem, xi1_pts);
      const auto nodal_normals = _amg.getNodalNormals(*msinfo.secondary_elem);

      const Elem * reinit_secondary_elem = secondary_ip;

      // If we're on the displaced mesh, we need to get the corresponding undisplaced elem before
      // calling _fe_problem.reinitElemFaceRef
      if (_displaced)
        reinit_secondary_elem = _fe_problem.mesh().elemPtr(reinit_secondary_elem->id());

      // reinit the variables/residuals/jacobians on the secondary interior
      _fe_problem.reinitElemFaceRef(
          reinit_secondary_elem, secondary_side_id, _secondary_boundary_id, TOLERANCE, &xi1_pts);

      const Elem * reinit_primary_elem = primary_ip;

      // If we're on the displaced mesh, we need to get the corresponding undisplaced elem before
      // calling _fe_problem.reinitElemFaceRef
      if (_displaced)
        reinit_primary_elem = _fe_problem.mesh().elemPtr(reinit_primary_elem->id());

      // reinit the variables/residuals/jacobians on the primary interior
      _fe_problem.reinitNeighborFaceRef(
          reinit_primary_elem, primary_side_id, _primary_boundary_id, TOLERANCE, &xi2_pts);

      // reinit neighbor materials, but be careful not to execute stateful materials since
      // conceptually they don't make sense with mortar (they're not interpolary)
      _fe_problem.reinitMaterialsNeighbor(primary_ip->subdomain_id(),
                                          /*tid=*/0,
                                          /*swap_stateful=*/false,
                                          /*execute_stateful=*/false);

      // reinit the variables/residuals/jacobians on the lower dimensional element corresponding to
      // the secondary face. This must be done last after the dof indices have been prepared for the
      // secondary (element) and primary (neighbor)
      _subproblem.reinitLowerDElem(secondary_face_elem, /*tid=*/0, &xi1_pts);

      // All this does currently is sets the neighbor/primary lower dimensional elem in Assembly and
      // computes its volume for potential use in the MortarConstraints. Solution continuity
      // stabilization for example relies on being able to access the volume
      _subproblem.reinitNeighborLowerDElem(msinfo.primary_elem);

      // reinit higher-dimensional secondary face/boundary materials. Do this after we reinit
      // lower-d variables in case we want to pull the lower-d variable values into the secondary
      // face/boundary materials. Be careful not to execute stateful materials since conceptually
      // they don't make sense with mortar (they're not interpolary)
      _fe_problem.reinitMaterialsFace(secondary_ip->subdomain_id(),
                                      /*tid=*/0,
                                      /*swap_stateful=*/false,
                                      /*execute_stateful=*/false);
      _fe_problem.reinitMaterialsBoundary(
          _secondary_boundary_id, /*tid=*/0, /*swap_stateful=*/false, /*execute_stateful=*/false);

      num_cached++;
      if (!_fe_problem.currentlyComputingJacobian())
      {
        for (auto * const mc : _mortar_constraints)
        {
          mc->setNormals(mc->interpolateNormals() ? normals : nodal_normals);
          mc->computeResidual();
        }

        _assembly.cacheResidual();
        _assembly.cacheResidualNeighbor();
        _assembly.cacheResidualLower();

        if (num_cached % 20 == 0)
          _assembly.addCachedResiduals();

      }
      else
      {
        for (auto * const mc : _mortar_constraints)
        {
          mc->setNormals(mc->interpolateNormals() ? normals : nodal_normals);
          mc->computeJacobian();
        }

        _assembly.cacheJacobianMortar();

        if (num_cached % 20 == 0)
          _assembly.addCachedJacobian();
      }
      // End loop over msm segments on secondary face elem
    }
    // End loop over (active) secondary elems
  }

  // Call any post operations for our mortar constraints
  for (auto * const mc : _mortar_constraints)
  {
    if (_amg.incorrectEdgeDropping())
      mc->incorrectEdgeDroppingPost(_amg.getInactiveLMNodes());
    else
      mc->post();

    mc->zeroInactiveLMDofs(_amg.getInactiveLMNodes(), _amg.getInactiveLMElems());
  }

  // Make sure any remaining cached residuals/Jacobians get added
  if (!_fe_problem.currentlyComputingJacobian())
    _assembly.addCachedResiduals();
  else
    _assembly.addCachedJacobian();
}

void
ComputeMortarFunctor::projectQPoints3d(const Elem * msm_elem,
                                       const Elem * primal_elem,
                                       const unsigned int sub_elem_ind,
                                       std::vector<Point> & q_pts)
{
  auto && msm_order = msm_elem->default_order();
  auto && msm_type = msm_elem->type();

  // Get normal to linearized element, could store and query but computation is easy
  Point e1 = msm_elem->point(0) - msm_elem->point(1);
  Point e2 = msm_elem->point(2) - msm_elem->point(1);
  const Point normal = e2.cross(e1).unit();

  // Get sub-elem (for second order meshes, otherwise trivial)
  const auto sub_elem = msm_elem->get_extra_integer(sub_elem_ind);
  const ElemType primal_type = primal_elem->type();

  auto get_sub_elem_inds = [primal_type, sub_elem]() -> std::array<unsigned int, 4> {
    switch (primal_type)
    {
      case TRI3:
        return {{0, 1, 2, /*dummy, out of range*/ 10}};
      case QUAD4:
        return {{0, 1, 2, 3}};
      case TRI6:
        switch (sub_elem)
        {
          case 0:
            return {{0, 3, 5, /*dummy, out of range*/ 10}};
          case 1:
            return {{3, 4, 5, /*dummy, out of range*/ 10}};
          case 2:
            return {{3, 1, 4, /*dummy, out of range*/ 10}};
          case 3:
            return {{5, 4, 2, /*dummy, out of range*/ 10}};
          default:
            mooseError("get_sub_elem_inds: Invalid sub_elem: ", sub_elem);
        }
      case QUAD9:
        switch (sub_elem)
        {
          case 0:
            return {{0, 4, 8, 7}};
          case 1:
            return {{4, 1, 5, 8}};
          case 2:
            return {{8, 5, 2, 6}};
          case 3:
            return {{7, 8, 6, 3}};
          default:
            mooseError("get_sub_elem_inds: Invalid sub_elem: ", sub_elem);
        }
      default:
        mooseError("get_sub_elem_inds: Face element type: ",
                   libMesh::Utility::enum_to_string<ElemType>(primal_type),
                   " invalid for 3D mortar");
    }
  };

  // Transforms quadrature point from first order sub-elements (in case of second-order)
  // to primal element
  auto transform_qp = [primal_type, sub_elem](const Real nu, const Real xi) {
    switch (primal_type)
    {
      case TRI3:
        return Point(nu, xi, 0);
      case QUAD4:
        return Point(nu, xi, 0);
      case TRI6:
        switch (sub_elem)
        {
          case 0:
            return Point(0.5 * nu, 0.5 * xi, 0);
          case 1:
            return Point(0.5 * (1 - xi), 0.5 * (nu + xi), 0);
          case 2:
            return Point(0.5 * (1 + nu), 0.5 * xi, 0);
          case 3:
            return Point(0.5 * nu, 0.5 * (1 + xi), 0);
          default:
            mooseError("get_sub_elem_inds: Invalid sub_elem: ", sub_elem);
        }
      case QUAD9:
        switch (sub_elem)
        {
          case 0:
            return Point(0.5 * (nu - 1), 0.5 * (xi - 1), 0);
          case 1:
            return Point(0.5 * (nu + 1), 0.5 * (xi - 1), 0);
          case 2:
            return Point(0.5 * (nu + 1), 0.5 * (xi + 1), 0);
          case 3:
            return Point(0.5 * (nu - 1), 0.5 * (xi + 1), 0);
          default:
            mooseError("get_sub_elem_inds: Invalid sub_elem: ", sub_elem);
        }
      default:
        mooseError("transform_qp: Face element type: ",
                   libMesh::Utility::enum_to_string<ElemType>(primal_type),
                   " invalid for 3D mortar");
    }
  };

  // Get sub-elem node indexes
  auto sub_elem_inds = get_sub_elem_inds();

  // Loop through quadrature points on msm_elem
  for (auto qp : make_range(_qrule_msm->n_points()))
  {
    // Get physical point on msm_elem to project
    Point x0;
    for (auto n : make_range(msm_elem->n_nodes()))
      x0 += Moose::fe_lagrange_2D_shape(
                msm_type, msm_order, n, static_cast<const TypeVector<Real> &>(_qrule_msm->qp(qp))) *
            msm_elem->point(n);

    // Use msm_elem quadrature point as initial guess
    // (will be correct for aligned meshes)
    Dual2 xi1{};
    xi1.value() = _qrule_msm->qp(qp)(0);
    xi1.derivatives()[0] = 1.0;
    Dual2 xi2{};
    xi2.value() = _qrule_msm->qp(qp)(1);
    xi2.derivatives()[1] = 1.0;
    VectorValue<Dual2> xi(xi1, xi2, 0);
    unsigned int current_iterate = 0, max_iterates = 10;

    // Project qp from mortar segments to first order sub-elements (elements in case of first order
    // geometry)
    do
    {
      VectorValue<Dual2> x1;
      for (auto n : make_range(primal_elem->n_vertices()))
        x1 += Moose::fe_lagrange_2D_shape(primal_type, FIRST, n, xi) *
              primal_elem->point(sub_elem_inds[n]);
      auto u = x1 - x0;
      VectorValue<Dual2> F(u(1) * normal(2) - u(2) * normal(1),
                           u(2) * normal(0) - u(0) * normal(2),
                           u(0) * normal(1) - u(1) * normal(0));

      if (MetaPhysicL::raw_value(F).norm() < 1e-12)
        break;

      RealEigenMatrix J(3, 2);
      J << F(0).derivatives()[0], F(0).derivatives()[1], F(1).derivatives()[0],
          F(1).derivatives()[1], F(2).derivatives()[0], F(2).derivatives()[1];
      RealEigenVector f(3);
      f << F(0).value(), F(1).value(), F(2).value();
      const RealEigenVector dxi = -J.colPivHouseholderQr().solve(f);

      xi(0) += dxi(0);
      xi(1) += dxi(1);
    } while (++current_iterate < max_iterates);

    if (current_iterate < max_iterates)
    {
      // Transfer quadrature point from sub-element to element and store
      q_pts.push_back(transform_qp(xi(0).value(), xi(1).value()));

      // The following checks if quadrature point falls in correct domain.
      // On small mortar segment elements with very distorted elements this can fail, instead of
      // erroring simply truncate quadrature point, these points typically have very small
      // contributions to integrals
      auto & qp_back = q_pts.back();
      if (primal_elem->type() == TRI3 || primal_elem->type() == TRI6)
      {
        if (qp_back(0) < 0 || qp_back(1) < 0 || qp_back(0) + qp_back(1) > 1)
        {
          mooseException("Quadrature point: ", qp_back, " out of bounds, truncating.");
        }
      }
      else if (primal_elem->type() == QUAD4 || primal_elem->type() == QUAD9)
      {
        if (qp_back(0) < -1 || qp_back(0) > 1 || qp_back(1) < -1 || qp_back(1) > 1)
        {
          mooseException("Quadrature point: ", qp_back, " out of bounds, truncating");
        }
      }
    }
    else
    {
      mooseError("Newton iteration for mortar quadrature mapping msm_elem: ",
                 msm_elem->id(),
                 " to elem: ",
                 primal_elem->id(),
                 " didn't converge. MSM element volume: ",
                 msm_elem->volume());
    }
  }
}
