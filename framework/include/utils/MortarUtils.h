//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Assembly.h"
#include "FEProblemBase.h"
#include "SubProblem.h"

#include "libmesh/quadrature.h"
#include "libmesh/elem.h"
#include "libmesh/point.h"

namespace Moose
{
namespace Mortar
{
void projectQPoints3d(const Elem * msm_elem,
                      const Elem * primal_elem,
                      unsigned int sub_elem_index,
                      const QBase & qrule_msm,
                      std::vector<Point> & q_pts);

template <typename Map, typename Consumers, typename ActionFunctor>
void
loopOverMortarSegments(const Map & secondary_elems_to_mortar_segments,
                       Assembly & assembly,
                       SubProblem & subproblem,
                       FEProblemBase & fe_problem,
                       const AutomaticMortarGeneration & amg,
                       const bool displaced,
                       const Consumers & consumers,
                       const ActionFunctor act)
{
  const auto & primary_secondary_boundary_id_pairs = amg.primarySecondaryBoundaryIDPairs();
  mooseAssert(primary_secondary_boundary_id_pairs.size() == 1,
              "We currently only support a single primary-secondary ID pair per mortar segment "
              "mesh. We can probably support this without too much trouble if you want to contact "
              "a MOOSE developer.");

  const auto primary_boundary_id = primary_secondary_boundary_id_pairs[0].first;
  const auto secondary_boundary_id = primary_secondary_boundary_id_pairs[0].second;

  // For 3D mortar get index for retrieving sub-element info
  unsigned int secondary_sub_elem_index = 0, primary_sub_elem_index = 0;
  if (amg.dim() == 3)
  {
    secondary_sub_elem_index = amg.mortarSegmentMesh().get_elem_integer_index("secondary_sub_elem");
    primary_sub_elem_index = amg.mortarSegmentMesh().get_elem_integer_index("primary_sub_elem");
  }

  // The mortar quadrature rule. Necessary for sizing the number of custom points for re-init'ing
  // the secondary interior, primary interior, and secondary face elements
  const auto & qrule_msm = assembly.qRuleMortar();

  // The element Jacobian times weights
  const auto & JxW_msm = assembly.jxWMortar();

  // Set required material properties
  std::set<unsigned int> needed_mat_props;
  for (const auto & consumer : consumers)
  {
    const auto & mp_deps = consumer->getMatPropDependencies();
    needed_mat_props.insert(mp_deps.begin(), mp_deps.end());
  }
  fe_problem.setActiveMaterialProperties(needed_mat_props, /*tid=*/0);

  // Loop through secondary elements, accumulating quadrature points for all corresponding mortar
  // segments
  for (const auto & elem_to_msm : secondary_elems_to_mortar_segments)
  {
    const Elem * secondary_face_elem = elem_to_msm.first;
    const auto & msm_elems = elem_to_msm.second;

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
      subproblem.reinitMortarElem(msm_elem);

      // Get a reference to the MortarSegmentInfo for this Elem.
      const MortarSegmentInfo & msinfo = amg.mortarSegmentMeshElemToInfo().at(msm_elem);

      if (msm_elem->dim() == 1)
      {
        for (unsigned int qp = 0; qp < qrule_msm->n_points(); qp++)
        {
          const Real eta = qrule_msm->qp(qp)(0);

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
        projectQPoints3d(msm_elem,
                         msinfo.secondary_elem,
                         secondary_sub_elem_index,
                         *qrule_msm,
                         secondary_xi_pts);
        projectQPoints3d(
            msm_elem, msinfo.primary_elem, primary_sub_elem_index, *qrule_msm, primary_xi_pts);
      }

      // If edge dropping case we need JxW on the msm to compute dual shape functions
      if (assembly.needDual())
        std::copy(std::begin(JxW_msm), std::end(JxW_msm), std::back_inserter(JxW));

#ifndef NDEBUG
      // Verify that the expected number of quadrature points have been inserted
      expected_length += qrule_msm->n_points();
      mooseAssert(secondary_xi_pts.size() == expected_length,
                  "Fewer than expected secondary quadrature points");
      mooseAssert(primary_xi_pts.size() == expected_length,
                  "Fewer than expected primary quadrature points");

      if (assembly.needDual())
        mooseAssert(JxW.size() == expected_length, "Fewer than expected JxW values computed");
#endif
    }

    // Reinit dual shape coeffs if dual shape functions needed
    if (assembly.needDual())
      assembly.reinitDual(secondary_face_elem, secondary_xi_pts, JxW);

    unsigned int n_segment = 0;

    // Loop through contributing msm elements, computing residual and Jacobian this time
    for (const auto msm_elem : msm_elems)
    {
      n_segment++;

      // These will hold quadrature points for each segment
      std::vector<Point> xi1_pts, xi2_pts;

      // Get a reference to the MortarSegmentInfo for this Elem.
      const MortarSegmentInfo & msinfo = amg.mortarSegmentMeshElemToInfo().at(msm_elem);

      // Set the secondary interior parent and side ids
      const Elem * secondary_ip = msinfo.secondary_elem->interior_parent();
      unsigned int secondary_side_id = secondary_ip->which_side_am_i(msinfo.secondary_elem);

      // Set the primary interior parent and side ids
      const Elem * primary_ip = msinfo.primary_elem->interior_parent();
      unsigned int primary_side_id = primary_ip->which_side_am_i(msinfo.primary_elem);

      // Compute a JxW for the actual mortar segment element (not the lower dimensional element on
      // the secondary face!)
      subproblem.reinitMortarElem(msm_elem);

      // Extract previously computed mapped quadrature points for secondary and primary face
      // elements
      const unsigned int start = (n_segment - 1) * qrule_msm->n_points();
      const unsigned int end = n_segment * qrule_msm->n_points();
      xi1_pts.insert(
          xi1_pts.begin(), secondary_xi_pts.begin() + start, secondary_xi_pts.begin() + end);
      xi2_pts.insert(xi2_pts.begin(), primary_xi_pts.begin() + start, primary_xi_pts.begin() + end);

      const Elem * reinit_secondary_elem = secondary_ip;

      // If we're on the displaced mesh, we need to get the corresponding undisplaced elem before
      // calling fe_problem.reinitElemFaceRef
      if (displaced)
        reinit_secondary_elem = fe_problem.mesh().elemPtr(reinit_secondary_elem->id());

      // NOTE to future developers: it can be tempting to try and change calls on fe_problem to
      // calls on subproblem because it seems wasteful to reinit data on both the reference and
      // displaced problems regardless of whether we are running on a "reference" or displaced
      // mortar mesh. But making such changes opens a can of worms. For instance, a user may define
      // constant material properties that are used by mortar constraints and not think about
      // whether they should set `use_displaced_mesh` for the material (and indeed the user may want
      // those constant properties usable by both reference and displaced consumer objects). If we
      // reinit that material and we haven't reinit'd it's assembly member, then we will get things
      // like segmentation faults. Moreover, one can easily imagine that a material may couple in
      // variables so we need to make sure those are reinit'd too. So even though it's inefficient,
      // it's safest to keep making calls on fe_problem instead of subproblem

      // reinit the variables/residuals/jacobians on the secondary interior
      fe_problem.reinitElemFaceRef(
          reinit_secondary_elem, secondary_side_id, secondary_boundary_id, TOLERANCE, &xi1_pts);

      const Elem * reinit_primary_elem = primary_ip;

      // If we're on the displaced mesh, we need to get the corresponding undisplaced elem before
      // calling fe_problem.reinitElemFaceRef
      if (displaced)
        reinit_primary_elem = fe_problem.mesh().elemPtr(reinit_primary_elem->id());

      // reinit the variables/residuals/jacobians on the primary interior
      fe_problem.reinitNeighborFaceRef(
          reinit_primary_elem, primary_side_id, primary_boundary_id, TOLERANCE, &xi2_pts);

      // reinit neighbor materials, but be careful not to execute stateful materials since
      // conceptually they don't make sense with mortar (they're not interpolary)
      fe_problem.reinitMaterialsNeighbor(primary_ip->subdomain_id(),
                                         /*tid=*/0,
                                         /*swap_stateful=*/false,
                                         /*execute_stateful=*/false);

      // reinit the variables/residuals/jacobians on the lower dimensional element corresponding to
      // the secondary face. This must be done last after the dof indices have been prepared for the
      // secondary (element) and primary (neighbor)
      subproblem.reinitLowerDElem(secondary_face_elem, /*tid=*/0, &xi1_pts);

      // All this does currently is sets the neighbor/primary lower dimensional elem in Assembly and
      // computes its volume for potential use in the MortarConstraints. Solution continuity
      // stabilization for example relies on being able to access the volume
      subproblem.reinitNeighborLowerDElem(msinfo.primary_elem);

      // reinit higher-dimensional secondary face/boundary materials. Do this after we reinit
      // lower-d variables in case we want to pull the lower-d variable values into the secondary
      // face/boundary materials. Be careful not to execute stateful materials since conceptually
      // they don't make sense with mortar (they're not interpolary)
      fe_problem.reinitMaterialsFace(secondary_ip->subdomain_id(),
                                     /*tid=*/0,
                                     /*swap_stateful=*/false,
                                     /*execute_stateful=*/false);
      fe_problem.reinitMaterialsBoundary(
          secondary_boundary_id, /*tid=*/0, /*swap_stateful=*/false, /*execute_stateful=*/false);

      act();

    } // End loop over msm segments on secondary face elem
  }   // End loop over (active) secondary elems
}
}
}
