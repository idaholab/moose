//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorFEInnerProductIntegralPostprocessor.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorFEInnerProductIntegralPostprocessor);

InputParameters
MFEMVectorFEInnerProductIntegralPostprocessor::validParams()
{
  InputParameters params = MFEMPostprocessor::validParams();
  params += MFEMBlockRestrictable::validParams();
  params.addClassDescription(
      "Calculates the integral of the inner product of two vector variables within a subdomain.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of optional scalar coefficient to scale integrand by.");
  params.addRequiredParam<VariableName>("primal_variable",
                                        "Name of the first vector variable in the inner product.");
  params.addRequiredParam<VariableName>("dual_variable",
                                        "Name of the second vector variable in the inner product.");
  return params;
}

MFEMVectorFEInnerProductIntegralPostprocessor::MFEMVectorFEInnerProductIntegralPostprocessor(
    const InputParameters & parameters)
  : MFEMPostprocessor(parameters),
    MFEMBlockRestrictable(parameters, getMFEMProblem().mesh().getMFEMParMesh()),
    _primal_var(getMFEMProblem().getProblemData().gridfunctions.GetRef(
        getParam<VariableName>("primal_variable"))),
    _dual_var(getMFEMProblem().getProblemData().gridfunctions.GetRef(
        getParam<VariableName>("dual_variable"))),
    _scalar_coef(getScalarCoefficient("coefficient")),
    _dual_var_coef(&_dual_var),
    _scaled_dual_var_coef(_scalar_coef, _dual_var_coef),
    _subdomain_integrator(_primal_var.ParFESpace())
{
  if (isSubdomainRestricted())
  {
    _subdomain_integrator.AddDomainIntegrator(
        new mfem::VectorFEDomainLFIntegrator(_scaled_dual_var_coef), getSubdomainMarkers());
  }
  else
  {
    _subdomain_integrator.AddDomainIntegrator(
        new mfem::VectorFEDomainLFIntegrator(_scaled_dual_var_coef));
  }
}

void
MFEMVectorFEInnerProductIntegralPostprocessor::execute()
{
  _subdomain_integrator.Assemble();
  _integral = _subdomain_integrator(_primal_var);
}

PostprocessorValue
MFEMVectorFEInnerProductIntegralPostprocessor::getValue() const
{
  return _integral;
}

#endif
