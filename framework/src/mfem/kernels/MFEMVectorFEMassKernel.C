#include "MFEMVectorFEMassKernel.h"

registerMooseObject("PlatypusApp", MFEMVectorFEMassKernel);

InputParameters
MFEMVectorFEMassKernel::validParams()
{
  InputParameters params = MFEMBilinearFormKernel::validParams();
  params.addClassDescription("The mass operator ($k u$), with the weak "
                             "form of $ (k \\phi_i, \\times u_h), to be added to an MFEM problem");

  params.addParam<std::string>("coefficient",
                               "Name of MFEM coefficient k to multiply the Laplacian by");

  return params;
}

MFEMVectorFEMassKernel::MFEMVectorFEMassKernel(const InputParameters & parameters)
  : MFEMBilinearFormKernel(parameters),
    _coef_name(getParam<std::string>("coefficient")),
    _coef(getMFEMProblem()._coefficients._scalars.Get(_coef_name))
{
}

mfem::BilinearFormIntegrator *
MFEMVectorFEMassKernel::createIntegrator()
{
  return new mfem::VectorFEMassIntegrator(*_coef);
}