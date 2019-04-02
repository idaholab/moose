//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeMortarFunctor.h"
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "Assembly.h"

#include "libmesh/fe_base.h"

template <ComputeStage compute_stage>
ComputeMortarFunctor<compute_stage>::ComputeMortarFunctor(
    std::vector<std::shared_ptr<MortarConstraintBase>> & mortar_constraints,
    const AutomaticMortarGeneration & amg,
    FEProblemBase & fe_problem,
    bool on_displaced)
  : _amg(amg),
    _fe_problem(fe_problem),
    _on_displaced(on_displaced),
    _assembly(_on_displaced ? _fe_problem.getDisplacedProblem()->assembly(0)
                            : _fe_problem.assembly(0)),
    _moose_parent_mesh(_on_displaced ? _fe_problem.getDisplacedProblem()->mesh()
                                     : _fe_problem.mesh()),
    _interior_dimension(_fe_problem.mesh().getMesh().mesh_dimension()),
    _msm_dimension(_interior_dimension - 1),
    _qrule_msm(_assembly.qRuleFace())
{
  // Constructor the mortar constraints we will later loop over
  if (compute_stage == ComputeStage::RESIDUAL)
  {
    for (auto mc : mortar_constraints)
      if (auto cmc = std::dynamic_pointer_cast<MortarConstraint<ComputeStage::RESIDUAL>>(mc))
        _mortar_constraints.push_back(cmc);
  }
  else if (compute_stage == ComputeStage::JACOBIAN)
    for (auto mc : mortar_constraints)
      if (auto cmc = std::dynamic_pointer_cast<MortarConstraint<ComputeStage::JACOBIAN>>(mc))
        _mortar_constraints.push_back(cmc);

  // Create the FE object that we will use for JxW
  {
    Order order_for_jxw_fe = _mesh.hasSecondOrderElements() ? SECOND : FIRST;
    _fe_for_jxw(FEBase::build(_msm_dimension, FEType(order_for_jxw_fe, LAGRANGE)));

    _fe_for_jxw->attach_quadrature_rule(_qrule_msm);

    _JxW_msm = &_fe_for_jxw->get_JxW();
  }

  // Initialize the variable solution and gradient data structures
  for (auto mc : _mortar_constraints)
  {
    _variables_needed_for_values.insert(mc.neededVariablesForValues().begin(),
                                        mc.neededVariablesForValues().end());
    _variables_needed_for_gradients.insert(mc.neededVariablesForGradients().begin(),
                                           mc.neededVariablesForGradients().end());
  }

  // Now determine and create the FE types we will need to reinit
}

void
ComputeMortarFunctor::operator()()
{
  _lambda.resize(_qrule_msm.n_points());
  _u_slave.resize(_qrule_msm.n_points());
  _u_master.resize(_qrule_msm.n_points());
  if (_need_primal_gradient)
  {
    _grad_u_slave.resize(_qrule_msm.n_points());
    _grad_u_master.resize(_qrule_msm.n_points());
  }

  // Array to hold custom quadrature point locations on the slave and master sides
  std::vector<Point> custom_xi1_pts, custom_xi2_pts;

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

    // Re-compute FE data on the mortar segment Elem (JxW_msm is the only thing we actually use.)
    _fe_for_jxw->reinit(msm_elem);

    // Compute custom integration points for the slave side
    custom_xi1_pts.resize(_qrule_msm->n_points());

    for (unsigned int qp = 0; qp < _qrule_msm->n_points(); qp++)
    {
      Real eta = _qrule_msm->qp(qp)(0);
      Real xi1_eta = 0.5 * (1 - eta) * msinfo.xi1_a + 0.5 * (1 + eta) * msinfo.xi1_b;
      custom_xi1_pts[qp] = xi1_eta;
    }

    // Compute custom integration points for the master side
    if (_has_master)
    {
      custom_xi2_pts.resize(_qrule_msm->n_points());
      for (unsigned int qp = 0; qp < _qrule_msm->n_points(); qp++)
      {
        Real eta = _qrule_msm->qp(qp)(0);
        Real xi2_eta = 0.5 * (1 - eta) * msinfo.xi2_a + 0.5 * (1 + eta) * msinfo.xi2_b;
        custom_xi2_pts[qp] = xi2_eta;
      }

      // Reinit the master interior FE on the side in question at the custom qps.
      _fe_master_interior_primal->reinit(master_ip, master_side_id, TOLERANCE, &custom_xi2_pts);
    }

    for (unsigned int qp = 0; qp < _qrule_msm->n_points(); qp++)
    {
      _lambda[qp] = 0;
      _u_slave[qp] = 0;
      _u_master[qp] = 0;
      if (_need_primal_gradient)
      {
        _grad_u_slave[qp] = 0;
        _grad_u_master[qp] = 0;
      }
    }
    computeSolutions();

    if (compute_stage == ComputeStage::RESIDUAL)
      computeElementResidual();
    else
      computeElementJacobian();
  }
}
