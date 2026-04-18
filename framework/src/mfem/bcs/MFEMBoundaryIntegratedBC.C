//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMBoundaryIntegratedBC.h"

registerMooseMFEMObject("MooseApp", BoundaryIntegratedBC);

namespace Moose::MFEM
{
InputParameters
BoundaryIntegratedBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription("Adds the boundary integrator to an MFEM problem for the linear form "
                             "$(f, v)_\\Omega$ "
                             "arising from the weak form of the forcing term $f$.");
  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "coefficient", "1.", "The coefficient which will be used in the integrated BC");
  return params;
}

BoundaryIntegratedBC::BoundaryIntegratedBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
BoundaryIntegratedBC::createLFIntegrator()
{
  return new mfem::BoundaryLFIntegrator(_coef);
}

} // namespace Moose::MFEM
#endif
