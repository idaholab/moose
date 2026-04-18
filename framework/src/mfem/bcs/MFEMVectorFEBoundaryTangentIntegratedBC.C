//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorFEBoundaryTangentIntegratedBC.h"

registerMooseMFEMObject("MooseApp", VectorFEBoundaryTangentIntegratedBC);

namespace Moose::MFEM
{
InputParameters
VectorFEBoundaryTangentIntegratedBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription("Adds the boundary integrator to an MFEM problem for the linear form "
                             "$(\\vec n \\times \\vec f, \\vec v)_{\\partial\\Omega}$");
  params.addParam<Moose::MFEM::VectorCoefficientName>(
      "vector_coefficient", "1. 1. 1.", "Vector coefficient used in the boundary integrator");
  return params;
}

VectorFEBoundaryTangentIntegratedBC::VectorFEBoundaryTangentIntegratedBC(
    const InputParameters & parameters)
  : IntegratedBC(parameters), _vec_coef(getVectorCoefficient("vector_coefficient"))
{
}

// Create MFEM integrator to apply to the RHS of the weak form. Ownership managed by the caller.
mfem::LinearFormIntegrator *
VectorFEBoundaryTangentIntegratedBC::createLFIntegrator()
{
  return new mfem::VectorFEBoundaryTangentLFIntegrator(_vec_coef);
}

} // namespace Moose::MFEM
#endif
