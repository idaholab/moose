//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionHDGKernel.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"

registerMooseObject("MooseApp", DiffusionHDGKernel);

InputParameters
DiffusionHDGKernel::validParams()
{
  auto params = HDGKernel::validParams();
  params += DiffusionHDGAssemblyHelper::validParams();
  params.addParam<MooseFunctorName>("source", 0, "Source for the diffusing species");
  params.addClassDescription("Implements the diffusion equation for a hybridized discretization");
  return params;
}

DiffusionHDGKernel::DiffusionHDGKernel(const InputParameters & parameters)
  : HDGKernel(parameters),
    DiffusionHDGAssemblyHelper(this, this, this, _sys, _aux_sys, _tid),
    _source(getFunctor<Real>("source"))
{
}

// For notation, please read "A superconvergent LDG-hybridizable Galerkin method for second-order
// elliptic problems" by Cockburn

void
DiffusionHDGKernel::onElement()
{
  resizeData();

  // Populate LM dof indices
  _lm_dof_indices = _lm_u_dof_indices;

  // Populate primal dof indices if we are computing the primal increment
  if (!preparingForSolve())
  {
    _primal_dof_indices = _qu_dof_indices;
    _primal_dof_indices.insert(
        _primal_dof_indices.end(), _u_dof_indices.begin(), _u_dof_indices.end());
  }

  // qu and u
  vectorVolumeResidual(0, _qu_sol, _u_sol, _JxW, *_qrule);
  scalarVolumeResidual(_vector_n_dofs, _qu_sol, _source, _JxW, *_qrule, _current_elem, _q_point);
  vectorVolumeJacobian(0, 0, _vector_n_dofs, _JxW, *_qrule);
  scalarVolumeJacobian(_vector_n_dofs, 0, _JxW, *_qrule);
}

void
DiffusionHDGKernel::onInternalSide()
{
  // qu, u, lm_u
  vectorFaceResidual(0, _lm_u_sol, _JxW_face, *_qrule_face, _normals);
  vectorFaceJacobian(0, 0, _JxW_face, *_qrule_face, _normals);
  scalarFaceResidual(_vector_n_dofs, _qu_sol, _u_sol, _lm_u_sol, _JxW_face, *_qrule_face, _normals);
  scalarFaceJacobian(_vector_n_dofs, 0, _vector_n_dofs, 0, _JxW_face, *_qrule_face, _normals);
  lmFaceResidual(0, _qu_sol, _u_sol, _lm_u_sol, _JxW_face, *_qrule_face, _normals);
  lmFaceJacobian(0, 0, _vector_n_dofs, 0, _JxW_face, *_qrule_face, _normals);
}
