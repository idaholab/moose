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

template<>
InputParameters validParams<NodalValueSampler>()
{
  InputParameters params = validParams<NodalVariableVectorPostprocessor>();

  params += validParams<SamplerBase>();

  return params;
}

NodalValueSampler::NodalValueSampler(const InputParameters & parameters) :
    NodalVariableVectorPostprocessor(parameters),
    SamplerBase(parameters, this, _communicator)
{
  std::vector<std::string> var_names(_coupled_moose_vars.size());
  _values.resize(_coupled_moose_vars.size());

  for (unsigned int i=0; i<_coupled_moose_vars.size(); i++)
    var_names[i] = _coupled_moose_vars[i]->name();

  // Initialize the datastructions in SamplerBase
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
  for (unsigned int i=0; i<_coupled_moose_vars.size(); i++)
    _values[i] = _coupled_moose_vars[i]->nodalSln()[_qp];

  SamplerBase::addSample(*_current_node, _current_node->id(), _values);
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

