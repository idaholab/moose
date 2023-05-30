//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementValueSampler.h"

// MOOSE includes
#include "MooseVariableFE.h"

// C++ includes
#include <numeric>

registerMooseObject("MooseApp", ElementValueSampler);

InputParameters
ElementValueSampler::validParams()
{
  InputParameters params = ElementVariableVectorPostprocessor::validParams();

  params.addClassDescription("Samples values of elemental variable(s).");

  params += SamplerBase::validParams();

  return params;
}

ElementValueSampler::ElementValueSampler(const InputParameters & parameters)
  : ElementVariableVectorPostprocessor(parameters), SamplerBase(parameters, this, _communicator)
{
  // ensure that variables are elemental, i.e., not scalar and and not nodal
  for (unsigned int i = 0; i < _coupled_moose_vars.size(); i++)
    if (_coupled_moose_vars[i]->feType().family == SCALAR || _coupled_moose_vars[i]->isNodal())
      paramError(
          "variable", "The variable '", _coupled_moose_vars[i]->name(), "' is not elemental.");

  std::vector<std::string> var_names(_coupled_moose_vars.size());
  _values.resize(_coupled_moose_vars.size());

  for (unsigned int i = 0; i < _coupled_moose_vars.size(); i++)
    var_names[i] = _coupled_moose_vars[i]->name();

  // Initialize the data structures in SamplerBase
  SamplerBase::setupVariables(var_names);
}

void
ElementValueSampler::initialize()
{
  SamplerBase::initialize();
}

void
ElementValueSampler::execute()
{
  unsigned int i_fe = 0, i_fv = 0;
  for (unsigned int i = 0; i < _coupled_moose_vars.size(); i++)
    if (_coupled_moose_vars[i]->isFV())
      _values[i] = _coupled_standard_fv_moose_vars[i_fv++]->getElementalValue(_current_elem);
    else
      _values[i] = _coupled_standard_moose_vars[i_fe++]->getElementalValue(_current_elem);

  SamplerBase::addSample(_current_elem->vertex_average(), _current_elem->id(), _values);
}

void
ElementValueSampler::finalize()
{
  SamplerBase::finalize();
}

void
ElementValueSampler::threadJoin(const UserObject & y)
{
  const ElementValueSampler & vpp = static_cast<const ElementValueSampler &>(y);

  SamplerBase::threadJoin(vpp);
}
