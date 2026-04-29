//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexDivAux.h"
#include "MFEMProblem.h"

registerMooseMFEMObject("MooseApp", ComplexDivAux);

namespace Moose::MFEM
{
InputParameters
ComplexDivAux::validParams()
{
  InputParameters params = ComplexAuxKernel::validParams();
  params.addClassDescription("Calculates the divergence of a complex H(div) conforming RT source "
                             "variable and stores the result"
                             " on an L2 conforming result complex auxvariable");
  ExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "source", "Vector H(div) Moose::MFEM::ComplexVariable to take the divergence of.");
  params.addParam<mfem::real_t>(
      "scale_factor_real", 1.0, "Real part of the factor to scale result auxvariable by.");
  params.addParam<mfem::real_t>(
      "scale_factor_imag", 0.0, "Imaginary part of the factor to scale result auxvariable by.");

  return params;
}

ComplexDivAux::ComplexDivAux(const InputParameters & parameters)
  : ComplexAuxKernel(parameters),
    _source_var_name(getParam<VariableName>("source")),
    _source_var(*getMFEMProblem().getComplexGridFunction(_source_var_name)),
    _scale_factor(getParam<mfem::real_t>("scale_factor_real"),
                  getParam<mfem::real_t>("scale_factor_imag")),
    _div(_source_var.ParFESpace(), _result_var.ParFESpace())
{
  _sequence = _source_var.GetSequence() + _result_var.GetSequence();
  _div.Assemble();
  _div.Finalize();
}

// Computes the auxvariable.
void
ComplexDivAux::execute()
{
  update();
  _div.AddMult(_source_var.real(), _result_var.real() = 0);
  _div.AddMult(_source_var.imag(), _result_var.imag() = 0);

  complexScale(_result_var, _scale_factor);
}

void
ComplexDivAux::update()
{
  if (long sequence = _source_var.GetSequence() + _result_var.GetSequence() > _sequence)
  {
    _sequence = sequence;
    _div.Update();
    _div.Assemble();
    _div.Finalize();
  }
}

} // namespace Moose::MFEM
#endif
