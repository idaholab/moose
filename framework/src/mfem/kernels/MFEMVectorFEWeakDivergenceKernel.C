#include "MFEMVectorFEWeakDivergenceKernel.h"

registerMooseObject("PlatypusApp", MFEMVectorFEWeakDivergenceKernel);

InputParameters
MFEMVectorFEWeakDivergenceKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("The weak divergence operator for the mixed form "
                             "$(k\\vec u, \\nabla q')$ with $\\vec u$ a vector FE "
                             "type, to be added to an MFEM problem");

  params.addParam<std::string>("coefficient", "Name of property k to use.");

  return params;
}

MFEMVectorFEWeakDivergenceKernel::MFEMVectorFEWeakDivergenceKernel(
    const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef_name(getParam<std::string>("coefficient")),
    _coef(getMFEMProblem().getProperties().getScalarProperty(_coef_name))
{
}

mfem::BilinearFormIntegrator *
MFEMVectorFEWeakDivergenceKernel::createIntegrator()
{
  return new mfem::VectorFEWeakDivergenceIntegrator(*_coef);
}
