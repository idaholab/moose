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

registerMooseMFEMObject("MooseApp", ComplexVectorPeriodAveragedPostprocessor);

namespace Moose::MFEM
{
InputParameters
ComplexVectorPeriodAveragedPostprocessor::validParams()
{
  InputParameters params = Postprocessor::validParams();
  params += BlockRestrictable::validParams();
  params.addClassDescription("Calculates the time average of the inner product between two "
                             "complex MFEM vector FE variables, scaled by an optional scalar "
                             "coefficient.");
  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "coefficient", "1.", "Name of optional scalar coefficient to scale integrand by.");
  ExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "primal_variable", "Name of the first complex vector variable in the inner product.");
  ExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "dual_variable", "Name of the second complex vector variable in the inner product.");
  return params;
}

ComplexVectorPeriodAveragedPostprocessor::ComplexVectorPeriodAveragedPostprocessor(
    const InputParameters & parameters)
  : Postprocessor(parameters),
    BlockRestrictable(
        parameters,
        getMFEMProblem().getMFEMVariableMesh(getParam<VariableName>("primal_variable"))),
    _l2_fec(getMFEMProblem()
                .getComplexGridFunction(getParam<VariableName>("primal_variable"))
                ->ParFESpace()
                ->GetMaxElementOrder(),
            getMesh().Dimension()),
    _scalar_test_fespace(const_cast<mfem::ParMesh *>(&getMesh()), &_l2_fec),
    _scalar_var(&_scalar_test_fespace),
    _scalar_coef(getScalarCoefficient("coefficient")),
    _primal_var_real_coef(
        getVectorCoefficientByName(getParam<VariableName>("primal_variable") + "_real")),
    _primal_var_imag_coef(
        getVectorCoefficientByName(getParam<VariableName>("primal_variable") + "_imag")),
    _dual_var_real_coef(
        getVectorCoefficientByName(getParam<VariableName>("dual_variable") + "_real")),
    _dual_var_imag_coef(
        getVectorCoefficientByName(getParam<VariableName>("dual_variable") + "_imag")),
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
ComplexVectorPeriodAveragedPostprocessor::execute()
{
  _scalar_var.ProjectCoefficient(_scalar_coef);
  _subdomain_integrator.Assemble();
  _integral = _subdomain_integrator(_scalar_var);
}

PostprocessorValue
ComplexVectorPeriodAveragedPostprocessor::getValue() const
{
  return _integral;
}

} // namespace Moose::MFEM
#endif
