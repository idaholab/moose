#include "MFEMDiffusionKernel.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMDiffusionKernel);

InputParameters
MFEMDiffusionKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "The Laplacian operator ($-k\\nabla \\cdot \\nabla u$), with the weak "
      "form of $ (k\\nabla \\phi_i, \\nabla u_h), to be added to an MFEM problem");

  params.addParam<std::string>("coefficient",
                               "Name of property for diffusion coefficient k to multiply "
                               "the Laplacian by");

  return params;
}

MFEMDiffusionKernel::MFEMDiffusionKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef_name(getParam<std::string>("coefficient")),
    // FIXME: The MFEM bilinear form can also handle vector and matrix
    // coefficients, so ideally we'd handle all three too.
    _coef(getMFEMProblem().getProperties().getScalarProperty(_coef_name))
{
}

mfem::BilinearFormIntegrator *
MFEMDiffusionKernel::createIntegrator()
{
  return new mfem::DiffusionIntegrator(_coef);
}
