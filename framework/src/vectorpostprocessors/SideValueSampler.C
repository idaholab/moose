//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideValueSampler.h"

// MOOSE includes
#include "MooseVariableFE.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", SideValueSampler);

InputParameters
SideValueSampler::validParams()
{
  InputParameters params = SideVectorPostprocessor::validParams();
  params += SamplerBase::validParams();
  params.addClassDescription("Sample variable(s) along a sideset, internal or external.");
  params.addRequiredCoupledVar(
      "variable", "The names of the variables that this VectorPostprocessor operates on");

  return params;
}

SideValueSampler::SideValueSampler(const InputParameters & parameters)
  : SideVectorPostprocessor(parameters), SamplerBase(parameters, this, _communicator)
{
  std::vector<std::string> var_names(_coupled_moose_vars.size());
  _values.resize(_coupled_moose_vars.size());

  for (unsigned int i = 0; i < _coupled_moose_vars.size(); i++)
  {
    var_names[i] = _coupled_moose_vars[i]->name();
    SamplerBase::checkForStandardFieldVariableType(_coupled_moose_vars[i]);
  }

  // Initialize the data structures in SamplerBase
  SamplerBase::setupVariables(var_names);
}

void
SideValueSampler::initialize()
{
  SamplerBase::initialize();
}

void
SideValueSampler::execute()
{
  for (unsigned int _qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    for (unsigned int i = 0; i < _coupled_moose_vars.size(); i++)
      _values[i] = (dynamic_cast<MooseVariable *>(_coupled_moose_vars[i]))->sln()[_qp];

    SamplerBase::addSample(_q_point[_qp], _current_elem->id(), _values);
  }
}

void
SideValueSampler::finalize()
{
  SamplerBase::finalize();
}

void
SideValueSampler::threadJoin(const UserObject & y)
{
  const auto & vpp = static_cast<const SideValueSampler &>(y);

  SamplerBase::threadJoin(vpp);
}
