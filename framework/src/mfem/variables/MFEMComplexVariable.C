#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexVariable.h"
#include "MooseVariableBase.h"
#include "MFEMProblem.h"
#include "MFEMFESpace.h"

registerMooseObject("MooseApp", MFEMComplexVariable);

InputParameters
MFEMComplexVariable::validParams()
{
  InputParameters params = MFEMObject::validParams();
  params.addRequiredParam<MFEMFESpaceName>("fespace",
                                           "The finite element space this variable is defined on.");
  params += MooseVariableBase::validParams();
  params.addClassDescription(
      "Class for adding complex MFEM variables to the problem (`mfem::ParComplexGridFunction`s).");
  params.registerBase("MooseVariableBase");
  params.registerSystemAttributeName("MooseVariableBase");

  return params;
}

MFEMComplexVariable::MFEMComplexVariable(const InputParameters & parameters)
  : MFEMObject(parameters),
    _fespace(getMFEMProblem().getMFEMObject<MFEMFESpace>("MFEMFESpace",
                                                         getParam<MFEMFESpaceName>("fespace"))),
    _cmplx_gridfunction(buildComplexGridFunction())
{
  *_cmplx_gridfunction = 0.0;
}

const std::shared_ptr<mfem::ParComplexGridFunction>
MFEMComplexVariable::buildComplexGridFunction()
{
  return std::make_shared<mfem::ParComplexGridFunction>(_fespace.getFESpace().get());
}

void
MFEMComplexVariable::declareCoefficients()
{
  if (getFESpace().isScalar())
  {
    getMFEMProblem().getCoefficients().declareScalar<mfem::GridFunctionCoefficient>(
        name() + "_real", &getComplexGridFunction()->real());
    getMFEMProblem().getCoefficients().declareVector<mfem::GradientGridFunctionCoefficient>(
        name() + "_grad_real", &getComplexGridFunction()->real());
    getMFEMProblem().getCoefficients().declareScalar<mfem::GridFunctionCoefficient>(
        name() + "_imag", &getComplexGridFunction()->imag());
    getMFEMProblem().getCoefficients().declareVector<mfem::GradientGridFunctionCoefficient>(
        name() + "_grad_imag", &getComplexGridFunction()->imag());
  }
  else
  {
    getMFEMProblem().getCoefficients().declareVector<mfem::VectorGridFunctionCoefficient>(
        name() + "_real", &getComplexGridFunction()->real());
    getMFEMProblem().getCoefficients().declareVector<mfem::CurlGridFunctionCoefficient>(
        name() + "_curl_real", &getComplexGridFunction()->real());
    getMFEMProblem().getCoefficients().declareScalar<mfem::DivergenceGridFunctionCoefficient>(
        name() + "_div_real", &getComplexGridFunction()->real());
    getMFEMProblem().getCoefficients().declareScalar<MFEMVectorMagnitudeCoefficient>(
        name() + "_real_mag",
        getMFEMProblem().getCoefficients().getVectorCoefficient(name() + "_real"));

    getMFEMProblem().getCoefficients().declareVector<mfem::VectorGridFunctionCoefficient>(
        name() + "_imag", &getComplexGridFunction()->imag());
    getMFEMProblem().getCoefficients().declareVector<mfem::CurlGridFunctionCoefficient>(
        name() + "_curl_imag", &getComplexGridFunction()->imag());
    getMFEMProblem().getCoefficients().declareScalar<mfem::DivergenceGridFunctionCoefficient>(
        name() + "_div_imag", &getComplexGridFunction()->imag());
    getMFEMProblem().getCoefficients().declareScalar<MFEMVectorMagnitudeCoefficient>(
        name() + "_imag_mag",
        getMFEMProblem().getCoefficients().getVectorCoefficient(name() + "_imag"));
  }
}

#endif
