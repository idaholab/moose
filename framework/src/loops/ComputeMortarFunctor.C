//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeMortarFunctor.h"
#include "SubProblem.h"
#include "Assembly.h"
#include "MortarConstraint.h"
#include "AutomaticMortarGeneration.h"
#include "MooseMesh.h"
#include "Assembly.h"

#include "libmesh/fe_base.h"
#include "libmesh/quadrature.h"
#include "libmesh/elem.h"
#include "libmesh/point.h"
#include "libmesh/mesh_base.h"

template <ComputeStage compute_stage>
ComputeMortarFunctor<compute_stage>::ComputeMortarFunctor(
    const std::vector<std::shared_ptr<MortarConstraintBase>> & mortar_constraints,
    const AutomaticMortarGeneration & amg,
    SubProblem & subproblem)
  : _amg(amg),
    _subproblem(subproblem),
    _assembly(_subproblem.assembly(0)),
    _qrule_msm(_assembly.qRuleMortar())
{
  // Constructor the mortar constraints we will later loop over
  if (compute_stage == ComputeStage::RESIDUAL)
  {
    for (auto && mc : mortar_constraints)
      if (auto && cmc =
              std::dynamic_pointer_cast<MortarConstraint<ComputeStage::RESIDUAL>>(mc).get())
        _mortar_constraints.push_back(cmc);
  }
  else if (compute_stage == ComputeStage::JACOBIAN)
    for (auto && mc : mortar_constraints)
      if (auto && cmc =
              std::dynamic_pointer_cast<MortarConstraint<ComputeStage::JACOBIAN>>(mc).get())
        _mortar_constraints.push_back(cmc);

  _master_boundary_id = _amg.master_slave_boundary_id_pairs[0].first;
  _slave_boundary_id = _amg.master_slave_boundary_id_pairs[0].second;
}

template <ComputeStage compute_stage>
void
ComputeMortarFunctor<compute_stage>::operator()()
{
  // Array to hold custom quadrature point locations on the slave and master sides
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
    Real elem_volume = msm_elem->volume();

    if (elem_volume < TOLERANCE)
      continue;

    // Get a reference to the MortarSegmentInfo for this Elem.
    const MortarSegmentInfo & msinfo = _amg.msm_elem_to_info.at(msm_elem);

    // There may be no contribution from the master side if it is not "in contact".
    bool has_slave = msinfo.slave_elem ? true : false;
    _has_master = msinfo.has_master();

    if (!has_slave)
      mooseError("Error, mortar segment has no slave element associated with it!");

    // Pointer to the interior parent.
    const Elem * slave_ip = msinfo.slave_elem->interior_parent();

    // Look up which side of the interior parent we are.
    unsigned int slave_side_id = slave_ip->which_side_am_i(msinfo.slave_elem);

    // The lower-dimensional slave side element associated with this
    // mortar segment is simply msinfo.slave_elem.
    const Elem * slave_face_elem = msinfo.slave_elem;

    // These only get initialized if there is a master Elem associated to this segment.
    const Elem * master_ip = libmesh_nullptr;
    unsigned int master_side_id = libMesh::invalid_uint;

    if (_has_master)
    {
      // Set the master interior parent and side ids.
      master_ip = msinfo.master_elem->interior_parent();
      master_side_id = master_ip->which_side_am_i(msinfo.master_elem);
    }

    // Compute a JxW for the actual mortar segment element (not the lower dimensional element on the
    // slave face!)
    _subproblem.reinitMortarElem(msm_elem);

    // Compute custom integration points for the slave side
    custom_xi1_pts.resize(_qrule_msm->n_points());

    for (unsigned int qp = 0; qp < _qrule_msm->n_points(); qp++)
    {
      Real eta = _qrule_msm->qp(qp)(0);
      Real xi1_eta = 0.5 * (1 - eta) * msinfo.xi1_a + 0.5 * (1 + eta) * msinfo.xi1_b;
      custom_xi1_pts[qp] = xi1_eta;
    }

    // reinit the variables/residuals/jacobians on the slave interior
    _subproblem.reinitElemFaceRef(
        slave_ip, slave_side_id, _slave_boundary_id, TOLERANCE, &custom_xi1_pts);

    if (_has_master)
    {
      //  Compute custom integration points for the master side
      custom_xi2_pts.resize(_qrule_msm->n_points());
      for (unsigned int qp = 0; qp < _qrule_msm->n_points(); qp++)
      {
        Real eta = _qrule_msm->qp(qp)(0);
        Real xi2_eta = 0.5 * (1 - eta) * msinfo.xi2_a + 0.5 * (1 + eta) * msinfo.xi2_b;
        custom_xi2_pts[qp] = xi2_eta;
      }

      // reinit the variables/residuals/jacobians on the master interior
      _subproblem.reinitNeighborFaceRef(
          master_ip, master_side_id, _master_boundary_id, TOLERANCE, &custom_xi2_pts);
    }

    // reinit the variables/residuals/jacobians on the lower dimensional element corresponding to
    // the slave face. This must be done last after the dof indices have been prepared for the slave
    // (element) and master (neighbor)
    _subproblem.reinitLowerDElemRef(slave_face_elem, &custom_xi1_pts);

    if (compute_stage == ComputeStage::RESIDUAL)
    {
      for (auto && mc : _mortar_constraints)
        mc->computeResidual(_has_master);

      _assembly.cacheResidual();
      _assembly.cacheResidualNeighbor();
      _assembly.cacheResidualLower();

      if (num_cached % 20 == 0)
        _assembly.addCachedResiduals();
    }
    else
    {
      for (auto && mc : _mortar_constraints)
        mc->computeJacobian(_has_master);

      // Cache SlaveSlave
      _assembly.cacheJacobian();

      // It doesn't appears that we have caching functions for these yet, or at least it's not used
      // in ComputeJacobianThread. I'll make sure to add/use them if these methods show up in
      // profiling
      //
      // Add SlaveMaster, MasterSlave, MasterMaster
      _assembly.addJacobianNeighbor();

      // Add LowerLower, LowerSlave, LowerMaster, SlaveLower, MasterLower
      _assembly.addJacobianLower();

      if (num_cached % 20 == 0)
        _assembly.addCachedJacobian();
    }
  } // end for loop over elements

  // Make sure any remaining cached residuals/Jacobians get added
  if (compute_stage == ComputeStage::RESIDUAL)
    _assembly.addCachedResiduals();
  else
    _assembly.addCachedJacobian();
}

template class ComputeMortarFunctor<ComputeStage::RESIDUAL>;
template class ComputeMortarFunctor<ComputeStage::JACOBIAN>;
