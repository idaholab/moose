//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexAuxKernel.h"
#include "MFEMProblem.h"

InputParameters
MFEMComplexAuxKernel::validParams()
{
  InputParameters params = MFEMExecutedObject::validParams();
  params.registerBase("AuxKernel");
  params.registerSystemAttributeName("MFEMAuxKernel");
  params.addClassDescription("Base class for MFEM objects that update auxiliary variables outside "
                             "of the main solve step.");
  params.addRequiredParam<AuxVariableName>("variable",
                                           "The name of the variable that this object applies to");
  return params;
}

MFEMComplexAuxKernel::MFEMComplexAuxKernel(const InputParameters & parameters)
  : MFEMExecutedObject(parameters),
    _result_var_name(getParam<AuxVariableName>("variable")),
    _result_var(*getMFEMProblem().getComplexGridFunction(_result_var_name))
{
}

std::set<std::string>
MFEMComplexAuxKernel::consumedVariableNames() const
{
  std::set<std::string> names;
  appendTypedParamIfValid<VariableName>(names, "source");
  appendTypedVectorParamIfValid<VariableName>(names, "source_variables");
  appendTypedParamIfValid<VariableName>(names, "first_source_vec");
  appendTypedParamIfValid<VariableName>(names, "second_source_vec");
  return names;
}

std::set<std::string>
MFEMComplexAuxKernel::producedVariableNames() const
{
  return {_result_var_name};
}

void
MFEMComplexAuxKernel::complexAdd(mfem::ParComplexGridFunction & a,
                                 const mfem::ParComplexGridFunction & b,
                                 const std::complex<mfem::real_t> scale)
{
  // a += scale * b

  // check that the parfespaces match
  if (a.ParFESpace() != b.ParFESpace())
    mooseError("MFEMComplexAuxKernel::complexAdd: ParFESpaces of input variables do not match.");

  a.real().Add(scale.real(), b.real());
  a.real().Add(-scale.imag(), b.imag());
  a.imag().Add(scale.imag(), b.real());
  a.imag().Add(scale.real(), b.imag());
}

void
MFEMComplexAuxKernel::complexScale(mfem::ParComplexGridFunction & a,
                                   const std::complex<mfem::real_t> scale)
{
  // a *= scale

  mfem::ParComplexGridFunction b(a.ParFESpace());
  static_cast<mfem::Vector &>(b) = a;

  complexAdd(a, b, scale - 1);
}

#endif
