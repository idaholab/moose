#ifdef MFEM_ENABLED

#include "MFEMDomainLFKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDomainLFKernel);

InputParameters
MFEMDomainLFKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(f, v)_\\Omega$ arising from the weak form of $f$.");
  params.addParam<MFEMScalarCoefficientName>("coefficient", "Name of function coefficient f.");
  return params;
}

MFEMDomainLFKernel::MFEMDomainLFKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef(getScalarCoefficient(getParam<MFEMScalarCoefficientName>("coefficient")))
{
}

mfem::LinearFormIntegrator *
MFEMDomainLFKernel::createLFIntegrator()
{
  return new mfem::DomainLFIntegrator(_coef);
}

#endif
