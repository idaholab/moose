#include "MFEMVectorFEDomainLFKernel.h"

registerMooseObject("PlatypusApp", MFEMVectorFEDomainLFKernel);

InputParameters
MFEMVectorFEDomainLFKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("A volumetric function ($f$), with the weak "
                             "form of $ (f, u_h), to be added to an MFEM problem");

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
