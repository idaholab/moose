#ifdef MFEM_ENABLED

#include "MFEMVectorFEDomainLFKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorFEDomainLFKernel);

InputParameters
MFEMVectorFEDomainLFKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the linear form "
                             "$(\\vec f, \\vec v)_\\Omega$ "
                             "arising from the weak form of the forcing term $\\vec f$.");
  params.addParam<MFEMVectorCoefficientName>(
      "vector_coefficient", 0, "The name of the vector coefficient f");

  return params;
}

MFEMVectorFEDomainLFKernel::MFEMVectorFEDomainLFKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _vec_coef(getVectorProperty(getParam<MFEMVectorCoefficientName>("vector_coefficient")))
{
}

mfem::LinearFormIntegrator *
MFEMVectorFEDomainLFKernel::createLFIntegrator()
{
  return new mfem::VectorFEDomainLFIntegrator(_vec_coef);
}

#endif
