#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexVariable.h"
#include "MooseVariableBase.h"
#include "MFEMProblem.h"
#include "MFEMFESpace.h"
#include "MFEMVectorMagnitudeCoefficient.h"

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
  const MFEMFESpace & mfem_fespace = getFESpace();
  const int cont_type = mfem_fespace.getFEC()->GetContType();
  if (mfem_fespace.isScalar())
  {
    getMFEMProblem().getCoefficients().declareScalar<mfem::GridFunctionCoefficient>(
        name() + "_real", &getComplexGridFunction()->real());
    getMFEMProblem().getCoefficients().declareScalar<mfem::GridFunctionCoefficient>(
        name() + "_imag", &getComplexGridFunction()->imag());
    // If gradient is well-defined on this variable, create auxiliary coefficient
    if (cont_type == mfem::FiniteElementCollection::CONTINUOUS)
    {
      getMFEMProblem().getCoefficients().declareVector<mfem::GradientGridFunctionCoefficient>(
          name() + "_real_grad", &getComplexGridFunction()->real());
      getMFEMProblem().getCoefficients().declareScalar<MFEMVectorMagnitudeCoefficient>(
          name() + "_real_grad_mag",
          getMFEMProblem().getCoefficients().getVectorCoefficient(name() + "_real_grad"));
      getMFEMProblem().getCoefficients().declareVector<mfem::GradientGridFunctionCoefficient>(
          name() + "_imag_grad", &getComplexGridFunction()->imag());
      getMFEMProblem().getCoefficients().declareScalar<MFEMVectorMagnitudeCoefficient>(
          name() + "_imag_grad_mag",
          getMFEMProblem().getCoefficients().getVectorCoefficient(name() + "_imag_grad"));
    }
  }
  else
  {
    getMFEMProblem().getCoefficients().declareVector<mfem::VectorGridFunctionCoefficient>(
        name() + "_real", &getComplexGridFunction()->real());
    getMFEMProblem().getCoefficients().declareScalar<MFEMVectorMagnitudeCoefficient>(
        name() + "_real_mag",
        getMFEMProblem().getCoefficients().getVectorCoefficient(name() + "_real"));
    getMFEMProblem().getCoefficients().declareVector<mfem::VectorGridFunctionCoefficient>(
        name() + "_imag", &getComplexGridFunction()->imag());
    getMFEMProblem().getCoefficients().declareScalar<MFEMVectorMagnitudeCoefficient>(
        name() + "_imag_mag",
        getMFEMProblem().getCoefficients().getVectorCoefficient(name() + "_imag"));
    // If curl is well-defined on this variable, create auxiliary coefficient
    if (cont_type == mfem::FiniteElementCollection::TANGENTIAL ||
        cont_type == mfem::FiniteElementCollection::CONTINUOUS)
    {
      getMFEMProblem().getCoefficients().declareVector<mfem::CurlGridFunctionCoefficient>(
          name() + "_real_curl", &getComplexGridFunction()->real());
      getMFEMProblem().getCoefficients().declareScalar<MFEMVectorMagnitudeCoefficient>(
          name() + "_real_curl_mag",
          getMFEMProblem().getCoefficients().getVectorCoefficient(name() + "_real_curl"));
      getMFEMProblem().getCoefficients().declareVector<mfem::CurlGridFunctionCoefficient>(
          name() + "_imag_curl", &getComplexGridFunction()->imag());
      getMFEMProblem().getCoefficients().declareScalar<MFEMVectorMagnitudeCoefficient>(
          name() + "_imag_curl_mag",
          getMFEMProblem().getCoefficients().getVectorCoefficient(name() + "_imag_curl"));
    }
    // If divergence is well-defined on this variable, create auxiliary coefficient
    if (cont_type == mfem::FiniteElementCollection::NORMAL ||
        cont_type == mfem::FiniteElementCollection::CONTINUOUS)
    {
      getMFEMProblem().getCoefficients().declareScalar<mfem::DivergenceGridFunctionCoefficient>(
          name() + "_real_div", &getComplexGridFunction()->real());
      getMFEMProblem().getCoefficients().declareScalar<mfem::DivergenceGridFunctionCoefficient>(
          name() + "_imag_div", &getComplexGridFunction()->imag());
    }
  }
}

#endif
