//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionHDGZeroFluxBC.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "DiffusionHDGKernel.h"

registerMooseObject("MooseApp", DiffusionHDGZeroFluxBC);

InputParameters
DiffusionHDGZeroFluxBC::validParams()
{
  auto params = HDGIntegratedBC::validParams();
  params += DiffusionHDGAssemblyHelper::validParams();
  params.addClassDescription("Implements a zero flux boundary condition for use with a hybridized "
                             "discretization of the diffusion equation");
  return params;
}

DiffusionHDGZeroFluxBC::DiffusionHDGZeroFluxBC(const InputParameters & parameters)
  : HDGIntegratedBC(parameters), DiffusionHDGAssemblyHelper(this, this, _sys, _aux_sys, _tid)
{
}

void
DiffusionHDGZeroFluxBC::onBoundary()
{
  resizeData(*this);

  // qu, u, lm_u
  vectorFaceResidual(*this, 0, _lm_u_sol);
  vectorFaceJacobian(*this, 0, 0);
  scalarFaceResidual(*this, _vector_n_dofs, _qu_sol, _u_sol, _lm_u_sol);
  scalarFaceJacobian(*this, _vector_n_dofs, 0, _vector_n_dofs, 0);
  lmFaceResidual(*this, 0, _qu_sol, _u_sol, _lm_u_sol);
  lmFaceJacobian(*this, 0, 0, _vector_n_dofs, 0);
}
