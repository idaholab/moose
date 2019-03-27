//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarConstraint.h"

// MOOSE includes
#include "Assembly.h"
#include "FEProblem.h"
#include "MooseVariable.h"
#include "NearestNodeLocator.h"
#include "PenetrationLocator.h"
#include "AutomaticMortarGeneration.h"

#include "libmesh/quadrature.h"

defineADBaseValidParams(
    MortarConstraint,
    MortarConstraintBase,
    params.addRequiredParam<BoundaryID>("master_boundary_id",
                                        "The id of the master boundary sideset.");
    params.addRequiredParam<BoundaryID>("slave_boundary_id",
                                        "The id of the slave boundary sideset.");
    params.addRequiredParam<SubdomainID>("master_subdomain_id", "The id of the master subdomain.");
    params.addRequiredParam<SubdomainID>("slave_subdomain_id", "The id of the slave subdomain.");
    params.registerRelationshipManagers("AugmentSparsityOnInterface");
    params.addRequiredParam<VariableName>("master_variable", "Variable on master surface");
    params.addParam<VariableName>("slave_variable", "Variable on master surface");
    params.addParam<bool>(
        "periodic",
        false,
        "Whether this constraint is going to be used to enforce a periodic condition. This has the "
        "effect of changing the normals vector for projection from outward to inward facing"););

template <ComputeStage compute_stage>
MortarConstraint<compute_stage>::MortarConstraint(const InputParameters & parameters)
  : MortarConstraintBase(parameters),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),

    _slave_id(getParam<BoundaryID>("slave_boundary_id")),
    _master_id(getParam<BoundaryID>("master_boundary_id")),
    _slave_subdomain_id(getParam<SubdomainID>("slave_subdomain_id")),
    _master_subdomain_id(getParam<SubdomainID>("master_subdomain_id")),
    _amg(_fe_problem.getMortarInterface(std::make_pair(_master_id, _slave_id),
                                        std::make_pair(_master_subdomain_id, _slave_subdomain_id))),
    _master_var(_subproblem.getStandardVariable(_tid, getParam<VariableName>("master_variable"))),
    _slave_var(
        isParamValid("slave_variable")
            ? _subproblem.getStandardVariable(_tid, getParam<VariableName>("slave_variable"))
            : _subproblem.getStandardVariable(_tid, getParam<VariableName>("master_variable"))),
    _primal_var_number(_master_var.number()),
    _lambda_var_number(_var.number()),
    _dof_map(_sys.dofMap()),
    _fe_type_primal(_master_var.feType()),
    _fe_type_lambda(_var.feType()),
    _interior_dimension(_fe_problem.mesh().getMesh().mesh_dimension()),
    _msm_dimension(_interior_dimension - 1),
    _fe_msm_primal(FEBase::build(_msm_dimension, _fe_type_primal)),
    _fe_msm_lambda(FEBase::build(_msm_dimension, _fe_type_lambda)),
    _fe_slave_interior_primal(FEBase::build(_interior_dimension, _fe_type_primal)),
    _fe_master_interior_primal(FEBase::build(_interior_dimension, _fe_type_primal)),
    _qrule_msm(_msm_dimension, SECOND),
    _JxW_msm(_fe_msm_primal->get_JxW()),
    _test(_fe_msm_lambda->get_phi()),
    _test_slave(_fe_slave_interior_primal->get_phi()),
    _test_master(_fe_master_interior_primal->get_phi()),
    _grad_test_slave(_fe_slave_interior_primal->get_dphi()),
    _grad_test_master(_fe_master_interior_primal->get_dphi()),
    _phys_points_slave(_fe_slave_interior_primal->get_xyz()),
    _phys_points_master(_fe_master_interior_primal->get_xyz()),
    _lm_offset(0),
    _need_primal_gradient(false),
    _periodic(adGetParam<bool>("periodic"))
{
  _fe_msm_primal->attach_quadrature_rule(&_qrule_msm);
  _amg.periodicConstraint(_periodic);
}

template <ComputeStage compute_stage>
void
MortarConstraint<compute_stage>::loopOverMortarMesh()
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

    // Get the lambda dof indices for the slave element side.
    _dof_map.dof_indices(slave_face_elem, _dof_indices_lambda, _lambda_var_number);

    // Offset the beginning of slave primal dofs by the length of the LM dof indices
    _slave_primal_offset = _dof_indices_lambda.size();

    // Get primal dof indices associated with the interior slave Elem.
    _dof_map.dof_indices(slave_ip, _dof_indices_slave_interior_primal, _primal_var_number);

    // Offset the beginning of master primal dofs by the combined length of the
    // LM and slave primal dofs
    _master_primal_offset = _slave_primal_offset + _dof_indices_slave_interior_primal.size();

    // These only get initialized if there is a master Elem associated to this segment.
    const Elem * master_ip = libmesh_nullptr;
    unsigned int master_side_id = libMesh::invalid_uint;

    if (_has_master)
    {
      // Set the master interior parent and side ids.
      master_ip = msinfo.master_elem->interior_parent();
      master_side_id = master_ip->which_side_am_i(msinfo.master_elem);

      // Get primal dof indices associated with the interior master Elem.
      _dof_map.dof_indices(master_ip, _dof_indices_master_interior_primal, _primal_var_number);
    }
    else
      _dof_indices_master_interior_primal.clear();

    // Re-compute FE data on the mortar segment Elem (JxW_msm is the only thing we actually use.)
    _fe_msm_primal->reinit(msm_elem);

    // Compute custom integration points for the slave side
    {
      custom_xi1_pts.resize(_qrule_msm.n_points());

      for (unsigned int qp = 0; qp < _qrule_msm.n_points(); qp++)
      {
        Real eta = _qrule_msm.qp(qp)(0);
        Real xi1_eta = 0.5 * (1 - eta) * msinfo.xi1_a + 0.5 * (1 + eta) * msinfo.xi1_b;
        custom_xi1_pts[qp] = xi1_eta;
      }

      // When you don't provide a weights array, the FE object uses
      // a dummy rule with all 1's. You should probably never use
      // the resulting JxW values for anything important.
      _fe_msm_lambda->reinit(slave_face_elem, &custom_xi1_pts);

      // Reinit the slave interior FE on the side in question at the custom qps.
      _fe_slave_interior_primal->reinit(slave_ip, slave_side_id, TOLERANCE, &custom_xi1_pts);
    }

    // Compute custom integration points for the master side
    if (_has_master)
    {
      custom_xi2_pts.resize(_qrule_msm.n_points());
      for (unsigned int qp = 0; qp < _qrule_msm.n_points(); qp++)
      {
        Real eta = _qrule_msm.qp(qp)(0);
        Real xi2_eta = 0.5 * (1 - eta) * msinfo.xi2_a + 0.5 * (1 + eta) * msinfo.xi2_b;
        custom_xi2_pts[qp] = xi2_eta;
      }

      // Reinit the master interior FE on the side in question at the custom qps.
      _fe_master_interior_primal->reinit(master_ip, master_side_id, TOLERANCE, &custom_xi2_pts);
    }

    for (unsigned int qp = 0; qp < _qrule_msm.n_points(); qp++)
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

template <ComputeStage compute_stage>
void
MortarConstraint<compute_stage>::computeSolutions()
{
  auto && current_solution = (*_sys.currentSolution());

  for (unsigned int qp = 0; qp < _qrule_msm.n_points(); ++qp)
  {
    for (unsigned int i = 0; i < _dof_indices_lambda.size(); ++i)
    {
      auto idx = _dof_indices_lambda[i];
      auto soln_local = current_solution(idx);

      _lambda[qp] += soln_local * _test[i][qp];
    }
    for (unsigned int i = 0; i < _dof_indices_slave_interior_primal.size(); ++i)
    {
      auto idx = _dof_indices_slave_interior_primal[i];
      auto soln_local = current_solution(idx);

      _u_slave[qp] += soln_local * _test_slave[i][qp];
      if (_need_primal_gradient)
        _grad_u_slave[qp] += soln_local * _grad_test_slave[i][qp];
    }
    for (unsigned int i = 0; i < _dof_indices_master_interior_primal.size(); ++i)
    {
      auto idx = _dof_indices_master_interior_primal[i];
      auto soln_local = current_solution(idx);

      _u_master[qp] += soln_local * _test_master[i][qp];
      if (_need_primal_gradient)
        _grad_u_master[qp] += soln_local * _grad_test_master[i][qp];
    }
  }
}

template <>
void
MortarConstraint<JACOBIAN>::computeSolutions()
{
  auto && current_solution = (*_sys.currentSolution());

  for (unsigned int qp = 0; qp < _qrule_msm.n_points(); ++qp)
  {
    for (unsigned int i = 0; i < _dof_indices_lambda.size(); ++i)
    {
      auto idx = _dof_indices_lambda[i];
      DualReal soln_local = current_solution(idx);
      soln_local.derivatives()[_lm_offset + i] = 1;

      _lambda[qp] += soln_local * _test[i][qp];
    }
    for (unsigned int i = 0; i < _dof_indices_slave_interior_primal.size(); ++i)
    {
      auto idx = _dof_indices_slave_interior_primal[i];
      DualReal soln_local = current_solution(idx);
      soln_local.derivatives()[_slave_primal_offset + i] = 1;

      _u_slave[qp] += soln_local * _test_slave[i][qp];
      if (_need_primal_gradient)
        _grad_u_slave[qp] += soln_local * _grad_test_slave[i][qp];
    }
    for (unsigned int i = 0; i < _dof_indices_master_interior_primal.size(); ++i)
    {
      auto idx = _dof_indices_master_interior_primal[i];
      DualReal soln_local = current_solution(idx);
      soln_local.derivatives()[_master_primal_offset + i] = 1;

      _u_master[qp] += soln_local * _test_master[i][qp];
      if (_need_primal_gradient)
        _grad_u_master[qp] += soln_local * _grad_test_master[i][qp];
    }
  }
}

template <ComputeStage compute_stage>
void
MortarConstraint<compute_stage>::computeElementResidual()
{
  DenseVector<Real> F_primal_slave(_dof_indices_slave_interior_primal.size());
  DenseVector<Real> F_primal_master(_dof_indices_master_interior_primal.size());
  DenseVector<Real> F_lambda_slave(_dof_indices_lambda.size());

  for (_qp = 0; _qp < _qrule_msm.n_points(); ++_qp)
  {
    // LM residuals
    for (_i = 0; _i < _dof_indices_lambda.size(); ++_i)
      F_lambda_slave(_i) += _JxW_msm[_qp] * computeQpResidual();

    // slave interior primal residuals
    for (_i = 0; _i < _dof_indices_slave_interior_primal.size(); ++_i)
      F_primal_slave(_i) += _JxW_msm[_qp] * computeQpResidualSide(Moose::Slave);

    // master interior primal residuals
    if (_has_master)
      for (_i = 0; _i < _dof_indices_master_interior_primal.size(); ++_i)
        F_primal_master(_i) += _JxW_msm[_qp] * computeQpResidualSide(Moose::Master);
  }

  _sys.getVector(_sys.residualVectorTag()).add_vector(F_lambda_slave, _dof_indices_lambda);
  _sys.getVector(_sys.residualVectorTag())
      .add_vector(F_primal_slave, _dof_indices_slave_interior_primal);
  _sys.getVector(_sys.residualVectorTag())
      .add_vector(F_primal_master, _dof_indices_master_interior_primal);
}

template <>
void
MortarConstraint<JACOBIAN>::computeElementResidual()
{
}

template <ComputeStage compute_stage>
void
MortarConstraint<compute_stage>::computeElementJacobian()
{
  // Local Jacobians
  DenseMatrix<Real> K_primal_slave_lambda_slave(_dof_indices_slave_interior_primal.size(),
                                                _dof_indices_lambda.size());
  DenseMatrix<Real> K_primal_master_lambda_slave(_dof_indices_master_interior_primal.size(),
                                                 _dof_indices_lambda.size());

  DenseMatrix<Real> K_lambda_slave_lambda_slave(_dof_indices_lambda.size(),
                                                _dof_indices_lambda.size());
  DenseMatrix<Real> K_lambda_slave_primal_slave(_dof_indices_lambda.size(),
                                                _dof_indices_slave_interior_primal.size());
  DenseMatrix<Real> K_lambda_slave_primal_master(_dof_indices_lambda.size(),
                                                 _dof_indices_master_interior_primal.size());

  // Derivative of LM residuals wrt all dofs
  std::vector<DualReal> lm_residuals(_dof_indices_lambda.size(), 0);
  // Derivative of slave primal residuals wrt all dofs
  std::vector<DualReal> slave_primal_residuals(_dof_indices_slave_interior_primal.size(), 0);
  // Derivative of master primal residuals wrt all dofs
  std::vector<DualReal> master_primal_residuals(_dof_indices_master_interior_primal.size(), 0);

  for (_qp = 0; _qp < _qrule_msm.n_points(); ++_qp)
  {
    for (_i = 0; _i < K_lambda_slave_primal_slave.m(); ++_i)
      lm_residuals[_i] += _JxW_msm[_qp] * computeQpResidual();

    for (_i = 0; _i < K_primal_slave_lambda_slave.m(); ++_i)
      slave_primal_residuals[_i] += _JxW_msm[_qp] * computeQpResidualSide(Moose::Slave);

    for (_i = 0; _i < K_primal_master_lambda_slave.m(); ++_i)
      master_primal_residuals[_i] += _JxW_msm[_qp] * computeQpResidualSide(Moose::Master);
  }

  for (unsigned int i = 0; i < _dof_indices_lambda.size(); ++i)
  {
    for (unsigned int j = 0; j < _dof_indices_lambda.size(); ++j)
      K_lambda_slave_lambda_slave(i, j) = lm_residuals[i].derivatives()[_lm_offset + j];
    for (unsigned int j = 0; j < _dof_indices_slave_interior_primal.size(); ++j)
      K_lambda_slave_primal_slave(i, j) = lm_residuals[i].derivatives()[_slave_primal_offset + j];
    for (unsigned int j = 0; j < _dof_indices_master_interior_primal.size(); ++j)
      K_lambda_slave_primal_master(i, j) = lm_residuals[i].derivatives()[_master_primal_offset + j];
  }

  for (unsigned int i = 0; i < _dof_indices_slave_interior_primal.size(); ++i)
    for (unsigned int j = 0; j < _dof_indices_lambda.size(); ++j)
      K_primal_slave_lambda_slave(i, j) = slave_primal_residuals[i].derivatives()[_lm_offset + j];

  for (unsigned int i = 0; i < _dof_indices_master_interior_primal.size(); ++i)
    for (unsigned int j = 0; j < _dof_indices_lambda.size(); ++j)
      K_primal_master_lambda_slave(i, j) = master_primal_residuals[i].derivatives()[_lm_offset + j];

  _sys.getMatrix(_sys.systemMatrixTag())
      .add_matrix(
          K_primal_slave_lambda_slave, _dof_indices_slave_interior_primal, _dof_indices_lambda);
  _sys.getMatrix(_sys.systemMatrixTag())
      .add_matrix(
          K_primal_master_lambda_slave, _dof_indices_master_interior_primal, _dof_indices_lambda);
  _sys.getMatrix(_sys.systemMatrixTag())
      .add_matrix(K_lambda_slave_lambda_slave, _dof_indices_lambda, _dof_indices_lambda);
  _sys.getMatrix(_sys.systemMatrixTag())
      .add_matrix(
          K_lambda_slave_primal_slave, _dof_indices_lambda, _dof_indices_slave_interior_primal);
  _sys.getMatrix(_sys.systemMatrixTag())
      .add_matrix(
          K_lambda_slave_primal_master, _dof_indices_lambda, _dof_indices_master_interior_primal);
}

template <>
void
MortarConstraint<RESIDUAL>::computeElementJacobian()
{
}

template <ComputeStage compute_stage>
void
MortarConstraint<compute_stage>::computeResidual()
{
  loopOverMortarMesh();
}

template <>
void
MortarConstraint<JACOBIAN>::computeResidual()
{
}

template <ComputeStage compute_stage>
void
MortarConstraint<compute_stage>::computeJacobian()
{
  loopOverMortarMesh();
}

template <>
void
MortarConstraint<RESIDUAL>::computeJacobian()
{
}

adBaseClass(MortarConstraint);
