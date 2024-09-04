#include "MFEMVectorFEMassKernel.h"

registerMooseObject("PlatypusApp", MFEMVectorFEMassKernel);

InputParameters
MFEMVectorFEMassKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("The mass operator ($k u$), with the weak "
                             "form of $ (k \\phi_i, \\times u_h), to be added to an MFEM problem");

  params.addParam<std::string>("coefficient", "Name of property k to multiply the Laplacian by");

  return params;
}

MFEMVectorFEMassKernel::MFEMVectorFEMassKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef_name(getParam<std::string>("coefficient")),
    // FIXME: The MFEM bilinear form can also handle vector and matrix
    // coefficients, so ideally we'd handle all three too.
    _coef(getMFEMProblem().getProperties().getScalarProperty(_coef_name))
{
}

mfem::BilinearFormIntegrator *
MFEMVectorFEMassKernel::createIntegrator()
{
  return new mfem::VectorFEMassIntegrator(_coef);
}
