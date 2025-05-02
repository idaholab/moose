#ifdef MFEM_ENABLED

#include "MFEMVectorFEWeakDivergenceKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorFEWeakDivergenceKernel);

InputParameters
MFEMVectorFEWeakDivergenceKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the mixed bilinear form "
      "$(-k\\vec u, \\vec\\nabla v)_\\Omega$ "
      "arising from the weak form of the divergence operator "
      "$\\vec \\nabla \\cdot (k\\vec u)$.");
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
MFEMVectorFEWeakDivergenceKernel::createBFIntegrator()
{
  return new mfem::VectorFEWeakDivergenceIntegrator(_coef);
}

#endif
