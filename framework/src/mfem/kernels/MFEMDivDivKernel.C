#ifdef MFEM_ENABLED

#include "MFEMDivDivKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDivDivKernel);

InputParameters
MFEMDivDivKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the bilinear form "
      "$(k\\vec\\nabla \\cdot \\vec u, \\vec\\nabla \\cdot \\vec v)_\\Omega$ "
      "arising from the weak form of the grad-div operator "
      "$-\\vec\\nabla \\left( k \\vec\\nabla \\cdot \\vec u \\right)$.");

  params.addParam<std::string>("coefficient", "Name of property k to multiply the Laplacian by");

  return params;
}

MFEMDivDivKernel::MFEMDivDivKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef_name(getParam<std::string>("coefficient")),
    // FIXME: The MFEM bilinear form can also handle vector and matrix
    // coefficients, so ideally we'd handle all three too.
    _coef(getMFEMProblem().getProperties().getScalarProperty(_coef_name))
{
}

mfem::BilinearFormIntegrator *
MFEMDivDivKernel::createBFIntegrator()
{
  return new mfem::DivDivIntegrator(_coef);
}

#endif
