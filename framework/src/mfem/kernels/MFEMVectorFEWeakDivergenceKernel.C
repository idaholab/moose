#include "MFEMVectorFEWeakDivergenceKernel.h"

InputParameters
MFEMVectorFEWeakDivergenceKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("The weak divergence operator for the mixed form "
                             "form of (σ u, ∇ V') $ (k\\vec u, \\nabla q') with u a vector FE "
                             "type, to be added to an MFEM problem");

  params.addParam<std::string>("coefficient", "Name of MFEM coefficient k to use.");

  return params;
}

MFEMVectorFEWeakDivergenceKernel::MFEMVectorFEWeakDivergenceKernel(
    const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef_name(getParam<std::string>("coefficient")),
    _coef(getMFEMProblem()._coefficients._scalars.Get(_coef_name))
{
}

mfem::BilinearFormIntegrator *
MFEMVectorFEWeakDivergenceKernel::createIntegrator()
{
  return new mfem::VectorFEWeakDivergenceIntegrator(*_coef);
}
