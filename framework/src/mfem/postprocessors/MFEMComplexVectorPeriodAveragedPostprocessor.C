//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexVectorPeriodAveragedPostprocessor.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMComplexVectorPeriodAveragedPostprocessor);

InputParameters
MFEMComplexVectorPeriodAveragedPostprocessor::validParams()
{
  InputParameters params = MFEMPostprocessor::validParams();
  params += MFEMBlockRestrictable::validParams();
  params.addClassDescription("Calculates the time average of the inner product between two "
                             "complex MFEM vector FE variables, scaled by an optional scalar "
                             "coefficient.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of optional scalar coefficient to scale integrand by.");
  params.addRequiredParam<VariableName>(
      "primal_variable", "Name of the first complex vector variable in the inner product.");
  params.addRequiredParam<VariableName>(
      "dual_variable", "Name of the second complex vector variable in the inner product.");
  return params;
}

MFEMComplexVectorPeriodAveragedPostprocessor::MFEMComplexVectorPeriodAveragedPostprocessor(
    const InputParameters & parameters)
  : MFEMPostprocessor(parameters),
    MFEMBlockRestrictable(parameters, getMFEMProblem().mesh().getMFEMParMesh()),
    _primal_var(getMFEMProblem().getProblemData().cmplx_gridfunctions.GetRef(
        getParam<VariableName>("primal_variable"))),
    _dual_var(getMFEMProblem().getProblemData().cmplx_gridfunctions.GetRef(
        getParam<VariableName>("dual_variable"))),
    _l2_fec(_primal_var.ParFESpace()->GetMaxElementOrder(),
            getMFEMProblem().mesh().getMFEMParMesh().Dimension()),
    _scalar_test_fespace(&getMFEMProblem().mesh().getMFEMParMesh(), &_l2_fec),
    _scalar_var(&_scalar_test_fespace),
    _scalar_coef(getScalarCoefficient("coefficient")),
    _primal_var_real_coef(&_primal_var.real()),
    _primal_var_imag_coef(&_primal_var.imag()),
    _dual_var_real_coef(&_dual_var.real()),
    _dual_var_imag_coef(&_dual_var.imag()),
    _real_inner_product_coef(_primal_var_real_coef, _dual_var_real_coef),
    _imag_inner_product_coef(_primal_var_imag_coef, _dual_var_imag_coef),
    _sum_coef(_real_inner_product_coef, _imag_inner_product_coef, 0.5, 0.5),
    _subdomain_integrator(&_scalar_test_fespace)
{
  if (isSubdomainRestricted())
    _subdomain_integrator.AddDomainIntegrator(new mfem::DomainLFIntegrator(_sum_coef),
                                              getSubdomainMarkers());
  else
    _subdomain_integrator.AddDomainIntegrator(new mfem::DomainLFIntegrator(_sum_coef));
}

void
MFEMComplexVectorPeriodAveragedPostprocessor::execute()
{
  _scalar_var.ProjectCoefficient(_scalar_coef);
  _subdomain_integrator.Assemble();
  _integral = _subdomain_integrator(_scalar_var);
}

PostprocessorValue
MFEMComplexVectorPeriodAveragedPostprocessor::getValue() const
{
  return _integral;
}

#endif
