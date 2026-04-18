//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorBoundaryIntegratedBC.h"

registerMooseMFEMObject("MooseApp", VectorBoundaryIntegratedBC);

namespace Moose::MFEM
{
InputParameters
VectorBoundaryIntegratedBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription("Adds the boundary integrator to an MFEM problem for the linear form "
                             "$(\\vec f, \\vec v)_{\\partial\\Omega}$");
  params.addParam<Moose::MFEM::VectorCoefficientName>(
      "vector_coefficient", "1. 1. 1.", "Vector coefficient used in the boundary integrator");
  return params;
}

VectorBoundaryIntegratedBC::VectorBoundaryIntegratedBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _vec_coef(getVectorCoefficient("vector_coefficient"))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
VectorBoundaryIntegratedBC::createLFIntegrator()
{
  return new mfem::VectorBoundaryLFIntegrator(_vec_coef);
}

} // namespace Moose::MFEM
#endif
