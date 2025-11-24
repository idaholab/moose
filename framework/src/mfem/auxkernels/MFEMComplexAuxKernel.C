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
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("AuxKernel");
  params.addClassDescription("Base class for MFEMGeneralUserObjects that update auxiliary "
                             "variables outside of the main solve step.");
  params.addRequiredParam<AuxVariableName>("variable",
                                           "The name of the variable that this object applies to");
  return params;
}

MFEMComplexAuxKernel::MFEMComplexAuxKernel(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _result_var_name(getParam<AuxVariableName>("variable")),
    _result_var(*getMFEMProblem().getProblemData().cmplx_gridfunctions.Get(_result_var_name))
{
}

void
MFEMComplexAuxKernel::complexAdd(mfem::ParComplexGridFunction & a,
                                 const mfem::ParComplexGridFunction & b,
                                 const std::complex<mfem::real_t> scale = {1.0, 0.0})
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
                                   const std::complex<mfem::real_t> scale = {1.0, 0.0})
{
  // a *= scale

  // Ownership of buf_var data will pass to a
  mfem::ParComplexGridFunction * buf_var = new mfem::ParComplexGridFunction(a.ParFESpace());
  buf_var->real() = 0.0;
  buf_var->imag() = 0.0;

  buf_var->real().Add(scale.real(), a.real());
  buf_var->real().Add(-scale.imag(), a.imag());
  buf_var->imag().Add(scale.imag(), a.real());
  buf_var->imag().Add(scale.real(), a.imag());

  a = *buf_var;
}

#endif
