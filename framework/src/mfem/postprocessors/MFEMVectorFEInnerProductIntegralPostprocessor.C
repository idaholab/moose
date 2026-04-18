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

registerMooseMFEMObject("MooseApp", VectorFEInnerProductIntegralPostprocessor);

namespace Moose::MFEM
{
InputParameters
VectorFEInnerProductIntegralPostprocessor::validParams()
{
  InputParameters params = Postprocessor::validParams();
  params += BlockRestrictable::validParams();
  params.addClassDescription(
      "Calculates the integral of the inner product of two vector variables within a subdomain.");
  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "coefficient", "1.", "Name of optional scalar coefficient to scale integrand by.");
  ExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "primal_variable", "Name of the first vector variable in the inner product.");
  ExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "dual_variable", "Name of the second vector variable in the inner product.");
  return params;
}

VectorFEInnerProductIntegralPostprocessor::VectorFEInnerProductIntegralPostprocessor(
    const InputParameters & parameters)
  : Postprocessor(parameters),
    BlockRestrictable(
        parameters,
        getMFEMProblem().getMFEMVariableMesh(getParam<VariableName>("primal_variable"))),
    _primal_var(*getMFEMProblem().getGridFunction(getParam<VariableName>("primal_variable"))),
    _scaled_dual_var_coef(getScalarCoefficient("coefficient"),
                          getVectorCoefficientByName(getParam<VariableName>("dual_variable"))),
    _subdomain_integrator(_primal_var.ParFESpace())
{
  if (isSubdomainRestricted())
    _subdomain_integrator.AddDomainIntegrator(
        new mfem::VectorFEDomainLFIntegrator(_scaled_dual_var_coef), getSubdomainMarkers());
  else
    _subdomain_integrator.AddDomainIntegrator(
        new mfem::VectorFEDomainLFIntegrator(_scaled_dual_var_coef));
}

void
VectorFEInnerProductIntegralPostprocessor::execute()
{
  _subdomain_integrator.Assemble();
  _integral = _subdomain_integrator(_primal_var);
}

PostprocessorValue
VectorFEInnerProductIntegralPostprocessor::getValue() const
{
  return _integral;
}

} // namespace Moose::MFEM
#endif
