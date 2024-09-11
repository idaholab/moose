#include "MFEMMassKernel.h"

registerMooseObject("PlatypusApp", MFEMMassKernel);

InputParameters
MFEMMassKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("The mass operator ($k u$), with the weak "
                             "form of $ (k \\phi_i, \\times u_h), to be added to an MFEM problem");

  params.addParam<std::string>("coefficient",
                               "Name of MFEM coefficient k to multiply the Laplacian by");

  return params;
}

MFEMMassKernel::MFEMMassKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef_name(getParam<std::string>("coefficient")),
    _coef(getMFEMProblem().getProperties().getScalarProperty(_coef_name))
{
}

mfem::BilinearFormIntegrator *
MFEMMassKernel::createIntegrator()
{
  return new mfem::MassIntegrator(_coef);
}