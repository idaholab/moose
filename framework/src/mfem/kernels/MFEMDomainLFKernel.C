#ifdef MFEM_ENABLED

#include "MFEMDomainLFKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDomainLFKernel);

InputParameters
MFEMDomainLFKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the linear form "
                             "$(f, v)_\\Omega$ "
                             "arising from the weak form of the forcing term $f$.");
  params.addParam<std::string>("coefficient", "Name of scalar coefficient $f$.");
  return params;
}

MFEMDomainLFKernel::MFEMDomainLFKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef_name(getParam<std::string>("coefficient")),
    _coef(getScalarCoefficient(_coef_name))
{
}

mfem::LinearFormIntegrator *
MFEMDomainLFKernel::createLFIntegrator()
{
  return new mfem::DomainLFIntegrator(_coef);
}

#endif
