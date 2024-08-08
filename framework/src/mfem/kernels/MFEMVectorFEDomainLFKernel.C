#include "MFEMVectorFEDomainLFKernel.h"

registerMooseObject("PlatypusApp", MFEMVectorFEDomainLFKernel);

InputParameters
MFEMVectorFEDomainLFKernel::validParams()
{
  InputParameters params = MFEMLinearFormKernel::validParams();
  params.addClassDescription("A volumetric function ($f$), with the weak "
                             "form of $ (f, u_h), to be added to an MFEM problem");

  params.addParam<std::string>("vector_coefficient", "Name of MFEM vector coefficient f.");

  return params;
}

MFEMVectorFEDomainLFKernel::MFEMVectorFEDomainLFKernel(const InputParameters & parameters)
  : MFEMLinearFormKernel(parameters),
    _vec_coef_name(getParam<std::string>("vector_coefficient")),
    _vec_coef(getMFEMProblem()._coefficients._vectors.Get(_vec_coef_name))
{
}

mfem::LinearFormIntegrator *
MFEMVectorFEDomainLFKernel::createIntegrator()
{
  return new mfem::VectorFEDomainLFIntegrator(*_vec_coef);
}