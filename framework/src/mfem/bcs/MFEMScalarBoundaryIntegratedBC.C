#ifdef MFEM_ENABLED

#include "MFEMScalarBoundaryIntegratedBC.h"

registerMooseObject("MooseApp", MFEMScalarBoundaryIntegratedBC);

InputParameters
MFEMScalarBoundaryIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription("Adds the boundary integrator to an MFEM problem for the linear form "
                             "$(f, v)_\\Omega$ "
                             "arising from the weak form of the forcing term $f$.");
  params.addRequiredParam<MFEMScalarCoefficientName>(
      "coefficient",
      "The coefficient which will be used in the integrated BC. A coefficient can be any of the "
      "following: a variable, an MFEM material property, a function, a post-processor, or a "
      "numeric value.");
  return params;
}

MFEMScalarBoundaryIntegratedBC::MFEMScalarBoundaryIntegratedBC(
    const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _coef_name(getParam<MFEMScalarCoefficientName>("coefficient")),
    _coef(getScalarCoefficient(_coef_name))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMScalarBoundaryIntegratedBC::createLFIntegrator()
{
  return new mfem::BoundaryLFIntegrator(_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMScalarBoundaryIntegratedBC::createBFIntegrator()
{
  return nullptr;
}

#endif
