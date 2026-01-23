//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNodalMaxValueId.h"

registerKokkosPostprocessor("MooseApp", KokkosNodalMaxValueId);

InputParameters
KokkosNodalMaxValueId::validParams()
{
  InputParameters params = KokkosNodalVariablePostprocessor::validParams();
  params.addClassDescription(
      "Finds the node ID with the maximum nodal value across all postprocessors.");
  return params;
}

KokkosNodalMaxValueId::KokkosNodalMaxValueId(const InputParameters & parameters)
  : KokkosNodalVariablePostprocessor(parameters)
{
}

void
KokkosNodalMaxValueId::initialize()
{
  allocateReductionBuffer(2);
}

void
KokkosNodalMaxValueId::finalize()
{
  Real proxy = _reduction_buffer(0);
  auto value = std::make_pair<dof_id_type, processor_id_type>(_reduction_buffer(1), processor_id());

  gatherProxyValueMax(proxy, value);

  if (processor_id() == value.second)
    for (auto & [node, id] : kokkosMesh().getContiguousNodeMap())
      if (id == value.first)
      {
        _node_id = node->id();
        break;
      }

  comm().broadcast(_node_id, value.second);
}

Real
KokkosNodalMaxValueId::getValue() const
{
  return _node_id;
}
