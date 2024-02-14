//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionHybridizedZeroFluxBC.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "DiffusionHybridizedKernel.h"

registerMooseObject("MooseApp", DiffusionHybridizedZeroFluxBC);

InputParameters
DiffusionHybridizedZeroFluxBC::validParams()
{
  auto params = HybridizedIntegratedBC::validParams();
  params += DiffusionHybridizedInterface::validParams();
  params.addClassDescription("Implements a zero flux boundary condition for use with a hybridized "
                             "discretization of the diffusion equation");
  return params;
}

DiffusionHybridizedZeroFluxBC::DiffusionHybridizedZeroFluxBC(const InputParameters & parameters)
  : HybridizedIntegratedBC(parameters),
    DiffusionHybridizedInterface(this, this, _sys, _aux_sys, _tid)
{
}

void
DiffusionHybridizedZeroFluxBC::onBoundary()
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
