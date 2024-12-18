#ifdef MFEM_ENABLED

#include "MFEMMassKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMMassKernel);

InputParameters
MFEMMassKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k u, v)_\\Omega$ "
                             "arising from the weak form of the mass operator "
                             "$ku$.");
  params.addParam<MFEMScalarCoefficientName>("coefficient",
                                             "Name of property for the mass coefficient k.");
  return params;
}

MFEMMassKernel::MFEMMassKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef_name(getParam<MFEMScalarCoefficientName>("coefficient")),
    _coef(getScalarProperty(_coef_name))
{
}

mfem::BilinearFormIntegrator *
MFEMMassKernel::createBFIntegrator()
{
  return new mfem::MassIntegrator(_coef);
}

#endif
