//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalValueSampler.h"

// MOOSE includes
#include "MooseVariableFE.h"

// C++ includes
#include <numeric>

registerMooseObject("MooseApp", NodalValueSampler);

InputParameters
NodalValueSampler::validParams()
{
  InputParameters params = NodalVariableVectorPostprocessor::validParams();

  params.addClassDescription("Samples values of nodal variable(s).");

  params += SamplerBase::validParams();

  return params;
}

NodalValueSampler::NodalValueSampler(const InputParameters & parameters)
  : NodalVariableVectorPostprocessor(parameters), SamplerBase(parameters, this, _communicator)
{
  // ensure that variables are nodal, i.e., not scalar and and not elemental
  for (unsigned int i = 0; i < _coupled_moose_vars.size(); i++)
    if (_coupled_moose_vars[i]->feType().family == SCALAR || !_coupled_moose_vars[i]->isNodal())
      paramError("variable", "The variable '", _coupled_moose_vars[i]->name(), "' is not nodal.");

  std::vector<std::string> var_names(_coupled_moose_vars.size());
  _values.resize(_coupled_moose_vars.size());
  _has_values.resize(_coupled_moose_vars.size());

  for (unsigned int i = 0; i < _coupled_moose_vars.size(); i++)
    var_names[i] = _coupled_moose_vars[i]->name();

  // Initialize the data structures in SamplerBase
  SamplerBase::setupVariables(var_names);
}

void
NodalValueSampler::initialize()
{
  SamplerBase::initialize();
}

void
NodalValueSampler::execute()
{
  // There may not be a nodal solution value at every node.  This
  // can happen if, for instance, you have a LINEAR, LAGRANGE
  // variable defined on a mesh with quadratic elements.
  //
  // We currently handle the following cases:
  // 1.) *none* of the coupled vars have values at the current node
  // 2.) *all* of the coupled vars have values at the current node
  //
  // If you have two different discretizations, you'll have to use two
  // separate NodalValueSampler objects to get their values.
  for (unsigned int i = 0; i < _coupled_standard_moose_vars.size(); i++)
  {
    const VariableValue & nodal_solution = _coupled_standard_moose_vars[i]->dofValues();

    if (nodal_solution.size() > 0)
    {
      _values[i] = nodal_solution[_qp];
      _has_values[i] = 1;
    }
    else
    {
      _values[i] = 0.; // arbitrary, will not be used
      _has_values[i] = 0;
    }
  }

  // Sum the number of values we had
  unsigned int num_values =
      std::accumulate(_has_values.begin(), _has_values.end(), 0, std::plus<unsigned int>());

  // If the number of values matches the number of available values,
  // call addSample() as normal.  If there are more than zero values
  // available but less than the number requested, throw an error.
  // Otherwise, num_values==0, and we can skip adding the sample
  // entirely without error.
  if (num_values == _has_values.size())
    SamplerBase::addSample(*_current_node, _current_node->id(), _values);

  else if (num_values != 0 && num_values < _has_values.size())
    mooseError("You must use separate NodalValueSampler objects for variables with different "
               "discretizations.");
}

void
NodalValueSampler::finalize()
{
  SamplerBase::finalize();
}

void
NodalValueSampler::threadJoin(const UserObject & y)
{
  const NodalValueSampler & vpp = static_cast<const NodalValueSampler &>(y);

  SamplerBase::threadJoin(vpp);
}
