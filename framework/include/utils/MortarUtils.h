//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Assembly.h"
#include "FEProblemBase.h"
#include "MaterialBase.h"
#include "MaterialWarehouse.h"

#include "libmesh/quadrature.h"
#include "libmesh/elem.h"
#include "libmesh/point.h"

namespace Moose
{
namespace Mortar
{
/**
 * 3D projection operator for mapping qpoints on mortar segments to secondary or primary elements
 * @param msm_elem The mortar segment element that we will be mapping quadrature points from
 * @param primal_elem The "persistent" mesh element (e.g. it exists on the simulation's MooseMesh)
 * that we will be mapping quadrature points for. This can be either an element on the secondary or
 * primary face
 * @param sub_elem_index We will call \p msm_elem->get_extra_integer(sub_elem_index) in the
 * implementation in order to determine which sub-element of the primal element the mortar segment
 * element corresponds to. This \p sub_elem_index should correspond to the secondary element index
 * if \p primal_elem is a secondary face element and the primary element index if \p primal_elem is
 * a primary face element
 * @param qrule_msm The rule that governs quadrature on the mortar segment element
 * @param q_pts The output of this function. This will correspond to the the (reference space)
 * quadrature points that we wish to evaluate shape functions, etc., at on the primal element
 */
void projectQPoints3d(const Elem * msm_elem,
                      const Elem * primal_elem,
                      unsigned int sub_elem_index,
                      const QBase & qrule_msm,
                      std::vector<Point> & q_pts);

/**
 * This method will loop over pairs of secondary elements and their corresponding mortar segments,
 * reinitialize all finite element shape functions, variables, and material properties, and then
 * call a provided action function for each mortar segment
 * @param secondary_elems_to_mortar_segments This is a container of iterators. Each iterator should
 * point to a pair. The first member of the pair should be a pointer to a secondary face element and
 * the second member of the pair should correspond to a container of mortar segment element pointers
 * that correspond to the secondary face element.
 * @param assembly The object we will to use to reinitalize finite element data
 * @param subproblem The object we will use to reinitialize variables
 * @param fe_problem The object we will use to reinitialize material properties
 * @param amg The mortar mesh generation object which holds all the mortar mesh data
 * @param displaced Whether the mortar mesh was built from a displaced parent mesh
 * @param consumers A container of objects that are going to be using all the data that we are
 * reinitializing within this function. This may be, for instance, a container of mortar constraints
 * or auxiliary kernels. This \p consumers parameter is important as it allows us to build up
 * variable and material property dependencies that we must make sure we reinit
 * @param act The action functor that we will call for each mortar segment after we have
 * reinitalized all of our prereq data. This functor may, for instance, call \p computeResidual or
 * \p computeJacobian on mortar constraints, or \p computeValue for an auxiliary kernel
 */
template <typename Iterators, typename Consumers, typename ActionFunctor>
void
loopOverMortarSegments(
    const Iterators & secondary_elems_to_mortar_segments,
    Assembly & assembly,
    SubProblem & subproblem,
    FEProblemBase & fe_problem,
    const AutomaticMortarGeneration & amg,
    const bool displaced,
    const Consumers & consumers,
    const THREAD_ID tid,
    const std::map<SubdomainID, std::deque<MaterialBase *>> & secondary_ip_sub_to_mats,
    const std::map<SubdomainID, std::deque<MaterialBase *>> & primary_ip_sub_to_mats,
    const std::deque<MaterialBase *> & secondary_boundary_mats,
    const ActionFunctor act,
    const bool reinit_mortar_user_objects)
{
  const auto & primary_secondary_boundary_id_pair = amg.primarySecondaryBoundaryIDPair();

  const auto primary_boundary_id = primary_secondary_boundary_id_pair.first;
  const auto secondary_boundary_id = primary_secondary_boundary_id_pair.second;

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
  std::unordered_set<unsigned int> needed_mat_props;
  for (const auto & consumer : consumers)
  {
    const auto & mp_deps = consumer->getMatPropDependencies();
    needed_mat_props.insert(mp_deps.begin(), mp_deps.end());
  }
  fe_problem.setActiveMaterialProperties(needed_mat_props, /*tid=*/tid);

  // Loop through secondary elements, accumulating quadrature points for all corresponding mortar
  // segments
  for (const auto elem_to_msm : secondary_elems_to_mortar_segments)
  {
    const Elem * secondary_face_elem = subproblem.mesh().getMesh().elem_ptr(elem_to_msm->first);
    // Set the secondary interior parent and side ids
    const Elem * secondary_ip = secondary_face_elem->interior_parent();
    unsigned int secondary_side_id = secondary_ip->which_side_am_i(secondary_face_elem);
    const auto & secondary_ip_mats =
        libmesh_map_find(secondary_ip_sub_to_mats, secondary_ip->subdomain_id());

    const auto & msm_elems = elem_to_msm->second;

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
      subproblem.reinitMortarElem(msm_elem, tid);

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
    // lindsayad: is there any need to make sure we do this on both reference and displaced?
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

      // Set the primary interior parent and side ids
      const Elem * primary_ip = msinfo.primary_elem->interior_parent();
      unsigned int primary_side_id = primary_ip->which_side_am_i(msinfo.primary_elem);
      const auto & primary_ip_mats =
          libmesh_map_find(primary_ip_sub_to_mats, primary_ip->subdomain_id());

      // Compute a JxW for the actual mortar segment element (not the lower dimensional element on
      // the secondary face!)
      subproblem.reinitMortarElem(msm_elem, tid);

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
          reinit_secondary_elem, secondary_side_id, TOLERANCE, &xi1_pts, nullptr, tid);

      const Elem * reinit_primary_elem = primary_ip;

      // If we're on the displaced mesh, we need to get the corresponding undisplaced elem before
      // calling fe_problem.reinitElemFaceRef
      if (displaced)
        reinit_primary_elem = fe_problem.mesh().elemPtr(reinit_primary_elem->id());

      // reinit the variables/residuals/jacobians on the primary interior
      fe_problem.reinitNeighborFaceRef(
          reinit_primary_elem, primary_side_id, TOLERANCE, &xi2_pts, nullptr, tid);

      // reinit neighbor materials, but be careful not to execute stateful materials since
      // conceptually they don't make sense with mortar (they're not interpolary)
      fe_problem.reinitMaterialsNeighbor(primary_ip->subdomain_id(),
                                         /*tid=*/tid,
                                         /*swap_stateful=*/false,
                                         &primary_ip_mats);

      // reinit the variables/residuals/jacobians on the lower dimensional element corresponding to
      // the secondary face. This must be done last after the dof indices have been prepared for the
      // secondary (element) and primary (neighbor)
      subproblem.reinitLowerDElem(secondary_face_elem, /*tid=*/tid, &xi1_pts);

      // All this does currently is sets the neighbor/primary lower dimensional elem in Assembly and
      // computes its volume for potential use in the MortarConstraints. Solution continuity
      // stabilization for example relies on being able to access the volume
      subproblem.reinitNeighborLowerDElem(msinfo.primary_elem, tid);

      // reinit higher-dimensional secondary face/boundary materials. Do this after we reinit
      // lower-d variables in case we want to pull the lower-d variable values into the secondary
      // face/boundary materials. Be careful not to execute stateful materials since conceptually
      // they don't make sense with mortar (they're not interpolary)
      fe_problem.reinitMaterialsFace(secondary_ip->subdomain_id(),
                                     /*tid=*/tid,
                                     /*swap_stateful=*/false,
                                     &secondary_ip_mats);
      fe_problem.reinitMaterialsBoundary(
          secondary_boundary_id, /*tid=*/tid, /*swap_stateful=*/false, &secondary_boundary_mats);

      if (reinit_mortar_user_objects)
        fe_problem.reinitMortarUserObjects(primary_boundary_id, secondary_boundary_id, displaced);

      act();

    } // End loop over msm segments on secondary face elem
  }   // End loop over (active) secondary elems
}

/**
 * This function creates containers of materials necessary to execute the mortar method for a
 * supplied set of consumers
 * @param consumers The objects that we're building the material dependencies for. This could be a
 * container of mortar constraints or a "mortar" auxiliary kernel for example
 * @param fe_problem The finite element problem that we'll be querying for material warehouses
 * @param tid The thread ID that we will use for pulling the material warehouse
 * @param secondary_ip_sub_to_mats A map from the secondary interior parent subdomain IDs to the
 * required secondary \em block materials we will need to evaluate for the consumers
 * @param primary_ip_sub_to_mats A map from the primary interior parent subdomain IDs to the
 * required primary \em block materials we will need to evaluate for the consumers
 * @param secondary_boundary_mats The secondary \em boundary materials we will need to evaluate for
 * the consumers
 */
template <typename Consumers>
void
setupMortarMaterials(const Consumers & consumers,
                     FEProblemBase & fe_problem,
                     const AutomaticMortarGeneration & amg,
                     const THREAD_ID tid,
                     std::map<SubdomainID, std::deque<MaterialBase *>> & secondary_ip_sub_to_mats,
                     std::map<SubdomainID, std::deque<MaterialBase *>> & primary_ip_sub_to_mats,
                     std::deque<MaterialBase *> & secondary_boundary_mats)
{
  secondary_ip_sub_to_mats.clear();
  primary_ip_sub_to_mats.clear();
  secondary_boundary_mats.clear();

  auto & mat_warehouse = fe_problem.getRegularMaterialsWarehouse();
  auto get_required_sub_mats =
      [&mat_warehouse, tid, &consumers](
          const SubdomainID sub_id,
          const Moose::MaterialDataType mat_data_type) -> std::deque<MaterialBase *>
  {
    if (mat_warehouse[mat_data_type].hasActiveBlockObjects(sub_id, tid))
    {
      auto & sub_mats = mat_warehouse[mat_data_type].getActiveBlockObjects(sub_id, tid);
      return MaterialBase::buildRequiredMaterials(consumers, sub_mats, /*allow_stateful=*/false);
    }
    else
      return {};
  };

  // Construct secondary *block* materials container
  const auto & secondary_ip_sub_ids = amg.secondaryIPSubIDs();
  for (const auto secondary_ip_sub : secondary_ip_sub_ids)
    secondary_ip_sub_to_mats.emplace(
        secondary_ip_sub, get_required_sub_mats(secondary_ip_sub, Moose::FACE_MATERIAL_DATA));

  // Construct primary *block* materials container
  const auto & primary_ip_sub_ids = amg.primaryIPSubIDs();
  for (const auto primary_ip_sub : primary_ip_sub_ids)
    primary_ip_sub_to_mats.emplace(
        primary_ip_sub, get_required_sub_mats(primary_ip_sub, Moose::NEIGHBOR_MATERIAL_DATA));

  // Construct secondary *boundary* materials container
  const auto & boundary_pr = amg.primarySecondaryBoundaryIDPair();
  const auto secondary_boundary = boundary_pr.second;
  if (mat_warehouse.hasActiveBoundaryObjects(secondary_boundary, tid))
  {
    auto & boundary_mats = mat_warehouse.getActiveBoundaryObjects(secondary_boundary, tid);
    secondary_boundary_mats =
        MaterialBase::buildRequiredMaterials(consumers, boundary_mats, /*allow_stateful=*/false);
  }
}
}
}
