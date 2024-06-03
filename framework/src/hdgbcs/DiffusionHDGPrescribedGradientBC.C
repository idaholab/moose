//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionHDGPrescribedGradientBC.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "DiffusionHDGKernel.h"

registerMooseObject("MooseApp", DiffusionHDGPrescribedGradientBC);

InputParameters
DiffusionHDGPrescribedGradientBC::validParams()
{
  auto params = HDGIntegratedBC::validParams();
  params += DiffusionHDGAssemblyHelper::validParams();
  params.addClassDescription("Implements a flux boundary condition for use with a hybridized "
                             "discretization of the diffusion equation");
  params.addParam<MooseFunctorName>(
      "normal_gradient", 0, "The prescribed value of the gradient dotted with the normal");
  return params;
}

DiffusionHDGPrescribedGradientBC::DiffusionHDGPrescribedGradientBC(
    const InputParameters & parameters)
  : HDGIntegratedBC(parameters),
    DiffusionHDGAssemblyHelper(this, this, this, _sys, _aux_sys, _tid),
    _normal_gradient(getFunctor<Real>("normal_gradient"))
{
}

void
DiffusionHDGPrescribedGradientBC::onBoundary()
{
  resizeData();

  // For notation, please read "A superconvergent LDG-hybridizable Galerkin method for second-order
  // elliptic problems" by Cockburn

  // qu, u, lm_u
  vectorFaceResidual(0, _lm_u_sol, _JxW_face, *_qrule_face, _normals);
  vectorFaceJacobian(0, 0, _JxW_face, *_qrule_face, _normals);
  scalarFaceResidual(_vector_n_dofs, _qu_sol, _u_sol, _lm_u_sol, _JxW_face, *_qrule_face, _normals);
  scalarFaceJacobian(_vector_n_dofs, 0, _vector_n_dofs, 0, _JxW_face, *_qrule_face, _normals);
  lmFaceResidual(0, _qu_sol, _u_sol, _lm_u_sol, _JxW_face, *_qrule_face, _normals);
  lmFaceJacobian(0, 0, _vector_n_dofs, 0, _JxW_face, *_qrule_face, _normals);

  for (const auto qp : make_range(_qrule_face->n_points()))
    for (const auto i : make_range(_lm_n_dofs))
      // prescribed normal gradient
      _LMVec(i) += _JxW_face[qp] * _diff[qp] * _lm_phi_face[i][qp] *
                   _normal_gradient(
                       Moose::ElemSideQpArg{
                           _current_elem, _current_side, qp, _qrule_face, _q_point_face[qp]},
                       determineState());
}
