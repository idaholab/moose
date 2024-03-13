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
  params.addParam<FunctionName>("source", 0, "Source for the diffusing species");
  params.addClassDescription("Implements the diffusion equation for a hybridized discretization");
  return params;
}

DiffusionHDGKernel::DiffusionHDGKernel(const InputParameters & parameters)
  : HDGKernel(parameters),
    DiffusionHDGAssemblyHelper(this, this, _sys, _aux_sys, _tid),
    _source(getFunction("source"))
{
}

void
DiffusionHDGKernel::onElement()
{
  resizeData(*this);

  // Populate LM dof indices
  _lm_dof_indices = _lm_u_dof_indices;

  // Populate primal dof indices if we are computing the primal increment
  if (!computingGlobalData())
  {
    _primal_dof_indices = _qu_dof_indices;
    _primal_dof_indices.insert(
        _primal_dof_indices.end(), _u_dof_indices.begin(), _u_dof_indices.end());
  }

  // qu and u
  vectorVolumeResidual(*this, 0, _qu_sol, _u_sol);
  scalarVolumeResidual(*this, _vector_n_dofs, _qu_sol, _source);
  vectorVolumeJacobian(*this, 0, 0, _vector_n_dofs);
  scalarVolumeJacobian(*this, _vector_n_dofs, 0);
}

void
DiffusionHDGKernel::onInternalSide()
{
  // qu, u, lm_u
  vectorFaceResidual(*this, 0, _lm_u_sol);
  vectorFaceJacobian(*this, 0, 0);
  scalarFaceResidual(*this, _vector_n_dofs, _qu_sol, _u_sol, _lm_u_sol);
  scalarFaceJacobian(*this, _vector_n_dofs, 0, _vector_n_dofs, 0);
  lmFaceResidual(*this, 0, _qu_sol, _u_sol, _lm_u_sol);
  lmFaceJacobian(*this, 0, 0, _vector_n_dofs, 0);
}
