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

#include "libmesh/fe_base.h"
#include "libmesh/quadrature.h"
#include "libmesh/elem.h"
#include "libmesh/point.h"
#include "libmesh/mesh_base.h"

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
    _qrule_msm(_assembly.qRuleMortar())
{
  // Construct the mortar constraints we will later loop over
  for (auto mc : mortar_constraints)
    _mortar_constraints.push_back(mc.get());

  _primary_boundary_id = _amg.primary_secondary_boundary_id_pairs[0].first;
  _secondary_boundary_id = _amg.primary_secondary_boundary_id_pairs[0].second;
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

  // Array to hold custom quadrature point locations on the secondary and primary sides
  std::vector<Point> custom_xi1_pts, custom_xi2_pts;

  unsigned int num_cached = 0;
  for (MeshBase::const_element_iterator
           el = _amg.mortar_segment_mesh.active_local_elements_begin(),
           end_el = _amg.mortar_segment_mesh.active_local_elements_end();
       el != end_el;
       ++el)
  {
    // Note: this is a mortar segment mesh Elem, *not* an Elem
    // from the original mesh or the side of an Elem from the
    // original mesh.
    const Elem * msm_elem = *el;

    // We may eventually allow zero-length segments in the
    // MSM. They are OK connectivity-wise, and they don't
    // contribute to the mortar segment integrals, but we don't
    // want to do reinit() on them or there will be a negative
    // Jacobian error.
    // NOTE: calling volume is fine here because elem_volume is only
    // used to check if it is very, very small. Distinction between
    // coordinates systems do not matter.
    Real elem_volume = msm_elem->volume();

    if (elem_volume < TOLERANCE)
      continue;

    // Get a reference to the MortarSegmentInfo for this Elem.
    const MortarSegmentInfo & msinfo = _amg.msm_elem_to_info.at(msm_elem);

    // There may be no contribution from the primary side if it is not "in contact".
    bool has_secondary = msinfo.secondary_elem ? true : false;
    _has_primary = msinfo.hasPrimary();

    if (!has_secondary)
      mooseError("Error, mortar segment has no secondary element associated with it!");

    // Pointer to the interior parent.
    const Elem * secondary_ip = msinfo.secondary_elem->interior_parent();

    // Look up which side of the interior parent we are.
    unsigned int secondary_side_id = secondary_ip->which_side_am_i(msinfo.secondary_elem);

    // The lower-dimensional secondary side element associated with this
    // mortar segment is simply msinfo.secondary_elem.
    const Elem * secondary_face_elem = msinfo.secondary_elem;

    // These only get initialized if there is a primary Elem associated to this segment.
    const Elem * primary_ip = libmesh_nullptr;
    unsigned int primary_side_id = libMesh::invalid_uint;

    if (_has_primary)
    {
      // Set the primary interior parent and side ids.
      primary_ip = msinfo.primary_elem->interior_parent();
      primary_side_id = primary_ip->which_side_am_i(msinfo.primary_elem);
    }

    // Compute a JxW for the actual mortar segment element (not the lower dimensional element on
    // the secondary face!)
    _subproblem.reinitMortarElem(msm_elem);

    // Compute custom integration points for the secondary side
    custom_xi1_pts.resize(_qrule_msm->n_points());

    for (unsigned int qp = 0; qp < _qrule_msm->n_points(); qp++)
    {
      Real eta = _qrule_msm->qp(qp)(0);
      Real xi1_eta = 0.5 * (1 - eta) * msinfo.xi1_a + 0.5 * (1 + eta) * msinfo.xi1_b;
      custom_xi1_pts[qp] = xi1_eta;
    }

    const Elem * reinit_secondary_elem = secondary_ip;

    // If we're on the displaced mesh, we need to get the corresponding undisplaced elem before
    // calling _fe_problem.reinitElemFaceRef
    if (_displaced)
      reinit_secondary_elem = _fe_problem.mesh().elemPtr(reinit_secondary_elem->id());

    // reinit the variables/residuals/jacobians on the secondary interior
    _fe_problem.reinitElemFaceRef(reinit_secondary_elem,
                                  secondary_side_id,
                                  _secondary_boundary_id,
                                  TOLERANCE,
                                  &custom_xi1_pts);

    if (_has_primary)
    {
      //  Compute custom integration points for the primary side
      custom_xi2_pts.resize(_qrule_msm->n_points());
      for (unsigned int qp = 0; qp < _qrule_msm->n_points(); qp++)
      {
        Real eta = _qrule_msm->qp(qp)(0);
        Real xi2_eta = 0.5 * (1 - eta) * msinfo.xi2_a + 0.5 * (1 + eta) * msinfo.xi2_b;
        custom_xi2_pts[qp] = xi2_eta;
      }

      const Elem * reinit_primary_elem = primary_ip;

      // If we're on the displaced mesh, we need to get the corresponding undisplaced elem before
      // calling _fe_problem.reinitElemFaceRef
      if (_displaced)
        reinit_primary_elem = _fe_problem.mesh().elemPtr(reinit_primary_elem->id());

      // reinit the variables/residuals/jacobians on the primary interior
      _fe_problem.reinitNeighborFaceRef(
          reinit_primary_elem, primary_side_id, _primary_boundary_id, TOLERANCE, &custom_xi2_pts);

      // reinit neighbor materials, but be careful not to execute stateful materials since
      // conceptually they don't make sense with mortar (they're not interpolary)
      _fe_problem.reinitMaterialsNeighbor(primary_ip->subdomain_id(),
                                          /*tid=*/0,
                                          /*swap_stateful=*/false,
                                          /*execute_stateful=*/false);
    }

    // reinit the variables/residuals/jacobians on the lower dimensional element corresponding to
    // the secondary face. This must be done last after the dof indices have been prepared for the
    // secondary (element) and primary (neighbor)
    _subproblem.reinitLowerDElem(secondary_face_elem, /*tid=*/0, &custom_xi1_pts);

    // reinit higher-dimensional secondary face/boundary materials. Do this after we reinit lower-d
    // variables in case we want to pull the lower-d variable values into the secondary
    // face/boundary materials. Be careful not to execute stateful materials since conceptually they
    // don't make sense with mortar (they're not interpolary)
    _fe_problem.reinitMaterialsFace(secondary_ip->subdomain_id(),
                                    /*tid=*/0,
                                    /*swap_stateful=*/false,
                                    /*execute_stateful=*/false);
    _fe_problem.reinitMaterialsBoundary(
        _secondary_boundary_id, /*tid=*/0, /*swap_stateful=*/false, /*execute_stateful=*/false);

    if (!_fe_problem.currentlyComputingJacobian())
    {
      for (auto && mc : _mortar_constraints)
        mc->computeResidual(_has_primary);

      _assembly.cacheResidual();
      _assembly.cacheResidualNeighbor();
      _assembly.cacheResidualLower();

      num_cached++;

      if (num_cached % 20 == 0)
        _assembly.addCachedResiduals();
    }
    else
    {
      for (auto && mc : _mortar_constraints)
        mc->computeJacobian(_has_primary);

      // Cache SecondarySecondary
      _assembly.cacheJacobian();

      // It doesn't appears that we have caching functions for these yet, or at least it's not
      // used in ComputeJacobianThread. I'll make sure to add/use them if these methods show up in
      // profiling
      //
      // Add SecondaryPrimary, PrimarySecondary, PrimaryPrimary
      _assembly.addJacobianNeighbor();

      // Add LowerLower, LowerSecondary, LowerPrimary, SecondaryLower, PrimaryLower
      _assembly.addJacobianLower();

      num_cached++;

      if (num_cached % 20 == 0)
        _assembly.addCachedJacobian();
    }
  } // end for loop over elements

  // Make sure any remaining cached residuals/Jacobians get added
  if (!_fe_problem.currentlyComputingJacobian())
    _assembly.addCachedResiduals();
  else
    _assembly.addCachedJacobian();
}
