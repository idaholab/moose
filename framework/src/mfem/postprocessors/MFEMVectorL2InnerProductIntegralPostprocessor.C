//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorL2InnerProductIntegralPostprocessor.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorL2InnerProductIntegralPostprocessor);

InputParameters
MFEMVectorL2InnerProductIntegralPostprocessor::validParams()
{
  InputParameters params = MFEMPostprocessor::validParams();
  params += MFEMBlockRestrictable::validParams();
  params.addClassDescription("Calculates integral of the L2 inner product of two variables in the domain");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of optional scalar coefficient to scale integrand by.");
  params.addRequiredParam<VariableName>("primal_variable",
                                        "Name of the first vector variable in the inner product.");
  params.addRequiredParam<VariableName>("dual_variable",
                                        "Name of the second vector variable in the inner product.");
  return params;
}

MFEMVectorL2InnerProductIntegralPostprocessor::MFEMVectorL2InnerProductIntegralPostprocessor(
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
    _subdomain_integrator.AddDomainIntegrator(
        new mfem::VectorDomainLFIntegrator(_scaled_dual_var_coef), getSubdomainMarkers());
  else
    _subdomain_integrator.AddDomainIntegrator(
        new mfem::VectorDomainLFIntegrator(_scaled_dual_var_coef));
}

void
MFEMVectorL2InnerProductIntegralPostprocessor::execute()
{
  _subdomain_integrator.Assemble();
  _integral = _subdomain_integrator(_primal_var);
}

PostprocessorValue
MFEMVectorL2InnerProductIntegralPostprocessor::getValue() const
{
  return _integral;
}

#endif
