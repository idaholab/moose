#ifdef MFEM_ENABLED

#include "MFEMVectorFEDomainLFKernel.h"

registerMooseObject("MooseApp", MFEMVectorFEDomainLFKernel);

InputParameters
MFEMVectorFEDomainLFKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the linear form "
                             "$(\\vec f, \\vec v)_\\Omega$ "
                             "arising from the weak form of the forcing term $\\vec f$.");
  params.addParam<FunctionName>("function", 0, "The name of the function f");

  return params;
}

MFEMVectorFEDomainLFKernel::MFEMVectorFEDomainLFKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _vec_coef(getMFEMProblem().getVectorFunctionCoefficient(getParam<FunctionName>("function")))
{
}

mfem::LinearFormIntegrator *
MFEMVectorFEDomainLFKernel::createIntegrator()
{
  return new mfem::VectorFEDomainLFIntegrator(*_vec_coef);
}

#endif
