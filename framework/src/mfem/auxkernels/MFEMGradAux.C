//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMGradAux.h"
#include "MFEMProblem.h"

registerMooseMFEMObject("MooseApp", GradAux);

namespace Moose::MFEM
{
InputParameters
GradAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Calculates the gradient of an H1 conforming source variable and stores the result"
      " on an H(curl) conforming ND result auxvariable");
  ExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "source", "Scalar H1 Moose::MFEM::Variable to take the gradient of.");
  params.addParam<mfem::real_t>("scale_factor", 1.0, "Factor to scale result auxvariable by.");
  return params;
}

GradAux::GradAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _source_var_name(getParam<VariableName>("source")),
    _source_var(*getMFEMProblem().getGridFunction(_source_var_name)),
    _scale_factor(getParam<mfem::real_t>("scale_factor")),
    _grad(_source_var.ParFESpace(), _result_var.ParFESpace())
{
  _sequence = _source_var.GetSequence() + _result_var.GetSequence();
  _grad.Assemble();
  _grad.Finalize();
}

// Computes the auxvariable.
void
GradAux::execute()
{
  update();
  _grad.AddMult(_source_var, _result_var = 0, _scale_factor);
}

void
GradAux::update()
{
  if (long sequence = _source_var.GetSequence() + _result_var.GetSequence() > _sequence)
  {
    _sequence = sequence;
    _grad.Update();
    _grad.Assemble();
    _grad.Finalize();
  }
}

} // namespace Moose::MFEM
#endif
