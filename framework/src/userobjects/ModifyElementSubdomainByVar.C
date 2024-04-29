//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ModifyElementSubdomainByVar.h"

registerMooseObject("MooseApp", ModifyElementSubdomainByVar);

InputParameters
ModifyElementSubdomainByVar::validParams()
{
  InputParameters params = ElementSubdomainModifier::validParams();

  params.addRequiredCoupledVar(
      "coupled_var", "Coupled variable whose value is used to calculate the desired subdomain ID.");
  params.addClassDescription(
      "Modify the element subdomain ID based on the provided variable value.");
  return params;
}

ModifyElementSubdomainByVar::ModifyElementSubdomainByVar(const InputParameters & parameters)
  : ElementSubdomainModifier(parameters), _v(coupledValue("coupled_var"))
{
}

SubdomainID
ModifyElementSubdomainByVar::computeSubdomainID()
{
  // Calculate the desired subdomain ID for the current element.
  Real val = 0.0;
  for (const auto qp : make_range(_qrule->n_points()))
    val += _v[qp] * _JxW[qp] * _coord[qp];
  val /= _current_elem_volume;

  SubdomainID sid = (unsigned int)(round(val));

  // Check if the target subdomain ID exists in the mesh. Use the nearest subdomain ID if the
  // target subdomain ID does not exist in the mesh.
  if (_mesh.meshSubdomains().find(sid) == _mesh.meshSubdomains().end())
  {
    auto it = _mesh.meshSubdomains().lower_bound(sid);
    SubdomainID nearest_id =
        it == _mesh.meshSubdomains().end() ? *_mesh.meshSubdomains().rbegin() : *it;

    // Store the target subdomain ID if it hasn't been previously requested.
    if (_void_sids.find(sid) == _void_sids.end())
    {
      mooseWarning("Requested subdomain ",
                   sid,
                   " does not exist. Subdomain ID ",
                   nearest_id,
                   " is assigned. Please ensure the passed variable falls within the range of the "
                   "mesh's subdomain IDs for the expected behavior.");
      _void_sids.insert(sid);
    }
    return nearest_id;
  }
  return sid;
}
