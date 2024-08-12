#include "MFEMDiffusionKernel.h"

registerMooseObject("PlatypusApp", MFEMDiffusionKernel);

InputParameters
MFEMDiffusionKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "The Laplacian operator ($-k\\nabla \\cdot \\nabla u$), with the weak "
      "form of $ (k\\nabla \\phi_i, \\nabla u_h), to be added to an MFEM problem");

  params.addParam<std::string>("coefficient",
                               "Name of MFEM coefficient k to multiply the Laplacian by");

  return params;
}

MFEMDiffusionKernel::MFEMDiffusionKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef_name(getParam<std::string>("coefficient")),
    _coef(getMFEMProblem()._coefficients._scalars.Get(_coef_name))
{
}

mfem::BilinearFormIntegrator *
MFEMDiffusionKernel::createIntegrator()
{
  return new mfem::DiffusionIntegrator(*_coef);
}