//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMElementAverageValue.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMElementAverageValue);

InputParameters
MFEMElementAverageValue::validParams()
{
  InputParameters params = MFEMPostprocessor::validParams();
  params += MFEMBlockRestrictable::validParams();
  MFEMExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "variable", "Name of the scalar variable to average.");
  params.addClassDescription("Computes the volumetric average of a scalar MFEM variable.");
  return params;
}

MFEMElementAverageValue::MFEMElementAverageValue(const InputParameters & parameters)
  : MFEMPostprocessor(parameters),
    MFEMBlockRestrictable(parameters,
                          getMFEMProblem().getMFEMVariableMesh(getParam<VariableName>("variable"))),
    _var(*getMFEMProblem().getGridFunction(getParam<VariableName>("variable"))),
    _one(1.0),
    _lf(_var.ParFESpace())
{
  if (isSubdomainRestricted())
    _lf.AddDomainIntegrator(new mfem::DomainLFIntegrator(_one), getSubdomainMarkers());
  else
    _lf.AddDomainIntegrator(new mfem::DomainLFIntegrator(_one));
}

void
MFEMElementAverageValue::execute()
{
  _lf.Assemble();

  const mfem::real_t integral = _lf(_var);

  // Compute the volume of the domain (or restricted subdomains) by integrating
  // the constant 1 projected onto the variable's FE space.
  mfem::ParGridFunction ones(_var.ParFESpace());
  ones.ProjectCoefficient(_one);
  const mfem::real_t volume = _lf(ones);

  _value = integral / volume;
}

PostprocessorValue
MFEMElementAverageValue::getValue() const
{
  return _value;
}

#endif
