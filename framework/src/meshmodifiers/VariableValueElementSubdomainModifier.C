//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableValueElementSubdomainModifier.h"

registerMooseObject("MooseApp", VariableValueElementSubdomainModifier);

InputParameters
VariableValueElementSubdomainModifier::validParams()
{
  InputParameters params = ElementSubdomainModifier::validParams();

  params.addRequiredCoupledVar(
      "coupled_var", "Coupled variable whose value is used to calculate the desired subdomain ID.");
  params.addClassDescription(
      "Modify the element subdomain ID based on the provided variable value.");
  return params;
}

VariableValueElementSubdomainModifier::VariableValueElementSubdomainModifier(
    const InputParameters & parameters)
  : ElementSubdomainModifier(parameters), _v(coupledValue("coupled_var"))
{
}

SubdomainID
VariableValueElementSubdomainModifier::computeSubdomainID()
{
  // Calculate the desired subdomain ID for the current element.
  Real val = 0.0;
  for (const auto qp : make_range(_qrule->n_points()))
    val += _v[qp] * _JxW[qp] * _coord[qp];
  val /= _current_elem_volume;

  SubdomainID sid = (unsigned int)(round(val));

  // Verify whether the specified target subdomain ID is present within the mesh. If it is not
  // found, locate the smallest subdomain ID in the mesh that matches or exceeds the target
  // subdomain ID. Or select the largest subdomain ID present in the mesh if all subdomain IDs are
  // smaller than the target.
  if (_mesh.meshSubdomains().find(sid) == _mesh.meshSubdomains().end())
  {
    auto it = _mesh.meshSubdomains().lower_bound(sid);
    SubdomainID lower_bound_id =
        it == _mesh.meshSubdomains().end() ? *_mesh.meshSubdomains().rbegin() : *it;

    // Store the target subdomain ID if it hasn't been previously requested.
    // Lock the _void_sids for thread-safe operations.
    std::lock_guard<std::mutex> lock(_void_sids_mutex);
    if (_void_sids.find(sid) == _void_sids.end())
    {
      mooseWarning("Requested subdomain ",
                   sid,
                   " does not exist. Subdomain ID ",
                   lower_bound_id,
                   " is assigned. Please ensure the passed variable falls within the range of the "
                   "mesh's subdomain IDs for the expected behavior.");
      _void_sids.insert(sid);
    }
    return lower_bound_id;
  }
  return sid;
}
