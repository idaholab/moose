//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReferenceElementJacobianDamper.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h" // _qrule

registerMooseObject("TensorMechanicsApp", ReferenceElementJacobianDamper);

InputParameters
ReferenceElementJacobianDamper::validParams()
{
  InputParameters params = GeneralDamper::validParams();
  params += BlockRestrictable::validParams();
  params.addClassDescription("Damper that limits the change in element Jacobians");
  params.addRequiredCoupledVar("displacements", "The nonlinear displacement variables");
  params.addParam<Real>(
      "max_increment", 0.1, "The maximum permissible incremental Jacobian per Newton iteration");
  return params;
}

ReferenceElementJacobianDamper::ReferenceElementJacobianDamper(const InputParameters & parameters)
  : GeneralDamper(parameters),
    Coupleable(this, /*nodal=*/true),
    BlockRestrictable(this),
    _max_jacobian_diff(getParam<Real>("max_increment")),
    _tid(getParam<THREAD_ID>("_tid")),
    _mesh(_subproblem.mesh()),
    _assembly(_subproblem.assembly(_tid)),
    _qrule(_assembly.qRule()),
    _ndisp(coupledComponents("displacements")),
    _disp_num(coupledIndices("displacements"))
{
  _grad_phi.resize(_ndisp);
  for (auto i : make_range(_ndisp))
  {
    const auto & disp_var = _sys.getFieldVariable<Real>(_tid, _disp_num[i]);
    checkVariable(disp_var);
    _grad_phi[i] = &disp_var.gradPhi();
  }
}

Real
ReferenceElementJacobianDamper::computeDamping(const NumericVector<Number> & solution,
                                               const NumericVector<Number> & update)
{
  // Maximum difference in the Jacobian for this Newton iteration
  Real max_difference = 0.0;

  // Loop over elements in the mesh
  for (const auto & elem : _mesh.getMesh().active_local_element_ptr_range())
  {
    if (!hasBlocks(elem->subdomain_id()))
      continue;

    // Compute gradients of displacements before and after this update
    computeGradDisp(elem, solution, update);

    // Compute the element Jacobian difference
    for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    {
      RankTwoTensor F;
      for (auto i : make_range(_ndisp))
        F.fillColumn(i, _grad_disp[i][qp]);
      F.addIa(1);

      RankTwoTensor F_update;
      for (auto i : make_range(_ndisp))
        F_update.fillColumn(i, _grad_disp_update[i][qp]);
      F_update.addIa(1);

      Real diff = std::abs(F_update.det() - F.det()) / F.det();
      if (diff > max_difference)
        max_difference = diff;
    }
  }

  _communicator.max(max_difference);

  if (max_difference > _max_jacobian_diff)
    return _max_jacobian_diff / max_difference;

  return 1.0;
}

void
ReferenceElementJacobianDamper::computeGradDisp(const Elem * elem,
                                                const NumericVector<Number> & solution,
                                                const NumericVector<Number> & update)
{
  // Reinit variable shape functions
  _assembly.setCurrentSubdomainID(elem->subdomain_id());
  _assembly.reinit(elem);

  _grad_disp.resize(_ndisp);
  _grad_disp_update.resize(_ndisp);
  for (auto i : make_range(_ndisp))
  {
    std::vector<dof_id_type> dof_indices;
    _sys.dofMap().dof_indices(elem, dof_indices, _disp_num[i]);
    _grad_disp[i].resize(_qrule->n_points());
    _grad_disp_update[i].resize(_qrule->n_points());
    for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    {
      _grad_disp[i][qp].zero();
      _grad_disp_update[i][qp].zero();
      for (auto dof_idx : index_range(dof_indices))
      {
        _grad_disp[i][qp] += (*_grad_phi[i])[dof_idx][qp] *
                             (solution(dof_indices[dof_idx]) + update(dof_indices[dof_idx]));
        _grad_disp_update[i][qp] += (*_grad_phi[i])[dof_idx][qp] * solution(dof_indices[dof_idx]);
      }
    }
  }
}
