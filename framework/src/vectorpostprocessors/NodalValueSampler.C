/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "NodalValueSampler.h"

// C++ includes
#include <numeric>

template <>
InputParameters
validParams<NodalValueSampler>()
{
  InputParameters params = validParams<NodalVariableVectorPostprocessor>();

  params += validParams<SamplerBase>();

  return params;
}

NodalValueSampler::NodalValueSampler(const InputParameters & parameters)
  : NodalVariableVectorPostprocessor(parameters), SamplerBase(parameters, this, _communicator)
{
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
  for (unsigned int i = 0; i < _coupled_moose_vars.size(); i++)
  {
    const VariableValue & nodal_solution = _coupled_moose_vars[i]->nodalSln();

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
