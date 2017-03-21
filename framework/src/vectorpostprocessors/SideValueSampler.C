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

#include "SideValueSampler.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<SideValueSampler>()
{
  InputParameters params = validParams<SideVectorPostprocessor>();

  params += validParams<SamplerBase>();

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
    var_names[i] = _coupled_moose_vars[i]->name();

  // Initialize the datastructions in SamplerBase
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
      _values[i] = _coupled_moose_vars[i]->sln()[_qp];

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
  const SideValueSampler & vpp = static_cast<const SideValueSampler &>(y);

  SamplerBase::threadJoin(vpp);
}
