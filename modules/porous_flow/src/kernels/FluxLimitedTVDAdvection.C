//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluxLimitedTVDAdvection.h"
#include "SystemBase.h"
#include "Assembly.h"

registerMooseObject("PorousFlowApp", FluxLimitedTVDAdvection);

InputParameters
FluxLimitedTVDAdvection::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Conservative form of $\\nabla \\cdot \\vec{v} u$ (advection), using "
                             "the Flux Limited TVD scheme invented by Kuzmin and Turek");
  params.addRequiredParam<UserObjectName>("advective_flux_calculator",
                                          "AdvectiveFluxCalculator UserObject");
  return params;
}

FluxLimitedTVDAdvection::FluxLimitedTVDAdvection(const InputParameters & parameters)
  : Kernel(parameters),
    _fluo(getUserObject<AdvectiveFluxCalculatorBase>("advective_flux_calculator"))
{
}

Real
FluxLimitedTVDAdvection::computeQpResidual()
{
  mooseError("FluxLimitedTVDAdvection::computeQpResidual() called\n");
  return 0.0;
}

void
FluxLimitedTVDAdvection::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());
  precalculateResidual();

  // get the residual contributions from _fluo
  for (unsigned i = 0; i < _current_elem->n_nodes(); ++i)
  {
    const dof_id_type node_id_i = _current_elem->node_id(i);
    _local_re(i) = _fluo.getFluxOut(node_id_i) / _fluo.getValence(node_id_i);
  }

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

void
FluxLimitedTVDAdvection::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());
  precalculateJacobian();

  // Run through the nodes of this element using "i", getting the Jacobian contributions
  // d(residual_i)/du(node_j) for all nodes j that can have a nonzero Jacobian contribution.  Some
  // of these node_j will live in this element, but some will live in other elements connected with
  // node "i", and some will live in the next layer of nodes (eg, in 1D residual_3 could have
  // contributions from node1, node2, node3, node4 and node5).
  for (unsigned i = 0; i < _current_elem->n_nodes(); ++i)
  {
    // global id of node "i"
    const dof_id_type node_id_i = _current_elem->node_id(i);
    // dof number of _var on node "i"
    std::vector<dof_id_type> idof_indices(
        1, _current_elem->node_ref(i).dof_number(_sys.number(), _var.number(), 0));
    // number of times node "i" is encountered in a sweep over elements
    const unsigned valence = _fluo.getValence(node_id_i);

    // retrieve the derivative information from _fluo
    const std::map<dof_id_type, Real> derivs = _fluo.getdFluxOutdu(node_id_i);

    // now build up the dof numbers of all the "j" nodes and the derivative matrix
    // d(residual_i)/d(u_j)
    std::vector<dof_id_type> jdof_indices(derivs.size());
    DenseMatrix<Number> deriv_matrix(1, derivs.size());
    unsigned j = 0;
    for (const auto & node_j_deriv : derivs)
    {
      // global id of j:
      const dof_id_type node_id_j = node_j_deriv.first;
      // dof at node j:
      jdof_indices[j] =
          _mesh.getMesh().node_ref(node_id_j).dof_number(_sys.number(), _var.number(), 0);
      // derivative must be divided by valence, otherwise the loop over elements will multiple-count
      deriv_matrix(0, j) = node_j_deriv.second / valence;
      j++;
    }
    // Add the result to the system's Jacobian matrix
    _assembly.cacheJacobianBlock(deriv_matrix, idof_indices, jdof_indices, _var.scalingFactor());
  }
}
