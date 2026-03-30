//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMSquareMagnitudeIntegralNormalizationPostprocessor.h"
#include "MFEMProblem.h"

#include <cmath>

registerMooseObject("MooseApp", MFEMSquareMagnitudeIntegralNormalizationPostprocessor);

InputParameters
MFEMSquareMagnitudeIntegralNormalizationPostprocessor::validParams()
{
  InputParameters params = MFEMPostprocessor::validParams();
  params += MFEMBlockRestrictable::validParams();
  params.addClassDescription(
      "Computes the L2 norm of a real or complex scalar MFEM variable over the domain (or "
      "specified subdomains) and writes the normalized variable to an auxiliary gridfunction. "
      "Accepts both real (H1, L2) and complex variables; the appropriate integration path is "
      "selected automatically. The postprocessor value is the normalization constant.");
  params.addRequiredParam<VariableName>("variable", "Name of the scalar variable to normalize.");
  params.addRequiredParam<VariableName>(
      "aux_variable",
      "Name of the auxiliary variable to write the normalized result into. Must share the same "
      "finite element space as 'variable'.");
  return params;
}

MFEMSquareMagnitudeIntegralNormalizationPostprocessor::
    MFEMSquareMagnitudeIntegralNormalizationPostprocessor(const InputParameters & parameters)
  : MFEMPostprocessor(parameters),
    MFEMBlockRestrictable(parameters, getMFEMProblem().mesh().getMFEMParMesh()),
    _is_complex(getMFEMProblem().getProblemData().cmplx_gridfunctions.Has(
        getParam<VariableName>("variable"))),
    _var(_is_complex
             ? nullptr
             : &getMFEMProblem().getProblemData().gridfunctions.GetRef(
                   getParam<VariableName>("variable"))),
    _cmplx_var(_is_complex
                   ? &getMFEMProblem().getProblemData().cmplx_gridfunctions.GetRef(
                         getParam<VariableName>("variable"))
                   : nullptr),
    _aux_var(_is_complex
                 ? nullptr
                 : &getMFEMProblem().getProblemData().gridfunctions.GetRef(
                       getParam<VariableName>("aux_variable"))),
    _cmplx_aux_var(_is_complex
                       ? &getMFEMProblem().getProblemData().cmplx_gridfunctions.GetRef(
                             getParam<VariableName>("aux_variable"))
                       : nullptr),
    _one(1.0)
{
  // Build the coefficient representing |u|^2, choosing the real or complex path.
  mfem::Coefficient * mag_sq_coef = nullptr;
  int fe_order = 0;

  if (_is_complex)
  {
    _ur_coef = std::make_unique<mfem::GridFunctionCoefficient>(&_cmplx_var->real());
    _ui_coef = std::make_unique<mfem::GridFunctionCoefficient>(&_cmplx_var->imag());
    _ur_sq_coef = std::make_unique<mfem::ProductCoefficient>(*_ur_coef, *_ur_coef);
    _ui_sq_coef = std::make_unique<mfem::ProductCoefficient>(*_ui_coef, *_ui_coef);
    _u_mag_sq_coef = std::make_unique<mfem::SumCoefficient>(*_ur_sq_coef, *_ui_sq_coef);
    mag_sq_coef = _u_mag_sq_coef.get();
    fe_order = _cmplx_var->ParFESpace()->GetMaxElementOrder();
  }
  else
  {
    _u_coef = std::make_unique<mfem::GridFunctionCoefficient>(_var);
    _u_sq_coef = std::make_unique<mfem::ProductCoefficient>(*_u_coef, *_u_coef);
    mag_sq_coef = _u_sq_coef.get();
    fe_order = _var->ParFESpace()->GetMaxElementOrder();
  }

  // Build a scalar L2 test space of matching order
  _l2_fec = std::make_unique<mfem::L2_FECollection>(
      fe_order, getMFEMProblem().mesh().getMFEMParMesh().Dimension());
  _test_fespace = std::make_unique<mfem::ParFiniteElementSpace>(
      &getMFEMProblem().mesh().getMFEMParMesh(), _l2_fec.get());
  _test_fn = std::make_unique<mfem::ParGridFunction>(_test_fespace.get());

  _integrator = std::make_unique<mfem::ParLinearForm>(_test_fespace.get());
  if (isSubdomainRestricted())
    _integrator->AddDomainIntegrator(new mfem::DomainLFIntegrator(*mag_sq_coef),
                                     getSubdomainMarkers());
  else
    _integrator->AddDomainIntegrator(new mfem::DomainLFIntegrator(*mag_sq_coef));
}

void
MFEMSquareMagnitudeIntegralNormalizationPostprocessor::execute()
{
  _test_fn->ProjectCoefficient(_one);
  _integrator->Assemble();
  _norm = std::sqrt((*_integrator)(*_test_fn));

  const mfem::real_t inv = 1.0 / _norm;

  if (_is_complex)
  {
    _cmplx_aux_var->real() = _cmplx_var->real();
    _cmplx_aux_var->real() *= inv;
    _cmplx_aux_var->imag() = _cmplx_var->imag();
    _cmplx_aux_var->imag() *= inv;
  }
  else
  {
    *_aux_var = *_var;
    *_aux_var *= inv;
  }
}

PostprocessorValue
MFEMSquareMagnitudeIntegralNormalizationPostprocessor::getValue() const
{
  return _norm;
}

#endif
