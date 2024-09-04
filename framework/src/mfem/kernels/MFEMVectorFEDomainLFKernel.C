#include "MFEMVectorFEDomainLFKernel.h"

registerMooseObject("PlatypusApp", MFEMVectorFEDomainLFKernel);

InputParameters
MFEMVectorFEDomainLFKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("A volumetric function ($f$), with the weak "
                             "form of $ (f, u_h), to be added to an MFEM problem");

  params.addParam<std::string>("vector_coefficient", "Name of vector property f.");

  return params;
}

MFEMVectorFEDomainLFKernel::MFEMVectorFEDomainLFKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _vec_coef_name(getParam<std::string>("vector_coefficient")),
    _vec_coef(getMFEMProblem().getProperties().getVectorProperty(_vec_coef_name))
{
}

mfem::LinearFormIntegrator *
MFEMVectorFEDomainLFKernel::createIntegrator()
{
  return new mfem::VectorFEDomainLFIntegrator(_vec_coef);
}
