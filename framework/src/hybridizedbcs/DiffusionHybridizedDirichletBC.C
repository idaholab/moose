//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionHybridizedDirichletBC.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "DiffusionHybridizedKernel.h"

registerMooseObject("MooseApp", DiffusionHybridizedDirichletBC);

InputParameters
DiffusionHybridizedDirichletBC::validParams()
{
  auto params = HybridizedIntegratedBC::validParams();
  params += DiffusionHybridizedInterface::validParams();
  params.addClassDescription("Weakly imposes Dirichlet boundary conditions for a "
                             "hybridized discretization of diffusion equation");
  params.addParam<FunctionName>("function", 0, "The Dirichlet value for the diffusing specie");
  return params;
}

DiffusionHybridizedDirichletBC::DiffusionHybridizedDirichletBC(const InputParameters & parameters)
  : HybridizedIntegratedBC(parameters),
    DiffusionHybridizedInterface(this, this, _sys, _aux_sys, _tid),
    _dirichlet_val(getFunction("function"))
{
}

void
DiffusionHybridizedDirichletBC::onBoundary()
{
  resizeData(*this);

  // qu, u
  vectorDirichletResidual(*this, 0, _dirichlet_val);
  scalarDirichletResidual(*this, _vector_n_dofs, _qu_sol, _u_sol, _dirichlet_val);
  scalarDirichletJacobian(*this, _vector_n_dofs, 0, _vector_n_dofs);

  // Set the LMs on these Dirichlet boundary faces to 0
  createIdentityResidual(_lm_phi_face, _lm_u_sol, _lm_n_dofs, 0);
  createIdentityJacobian(_lm_phi_face, _lm_n_dofs, 0);
}
