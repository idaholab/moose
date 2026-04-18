//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDivAux.h"
#include "MFEMProblem.h"

registerMooseMFEMObject("MooseApp", DivAux);

namespace Moose::MFEM
{
InputParameters
DivAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Calculates the divergence of an H(div) conforming RT source variable and stores the result"
      " on an L2 conforming result auxvariable");
  ExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "source", "Vector H(div) Moose::MFEM::Variable to take the divergence of.");
  params.addParam<mfem::real_t>("scale_factor", 1.0, "Factor to scale result auxvariable by.");
  return params;
}

DivAux::DivAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _source_var_name(getParam<VariableName>("source")),
    _source_var(*getMFEMProblem().getGridFunction(_source_var_name)),
    _scale_factor(getParam<mfem::real_t>("scale_factor")),
    _div(_source_var.ParFESpace(), _result_var.ParFESpace())
{
  _sequence = _source_var.GetSequence() + _result_var.GetSequence();
  _div.Assemble();
  _div.Finalize();
}

// Computes the auxvariable.
void
DivAux::execute()
{
  update();
  _div.AddMult(_source_var, _result_var = 0, _scale_factor);
}

void
DivAux::update()
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
