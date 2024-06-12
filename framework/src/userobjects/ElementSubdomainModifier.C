//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementSubdomainModifier.h"

InputParameters
ElementSubdomainModifier::validParams()
{
  InputParameters params = ElementSubdomainModifierBase::validParams();
  return params;
}

ElementSubdomainModifier::ElementSubdomainModifier(const InputParameters & parameters)
  : ElementSubdomainModifierBase(parameters)
{
}

void
ElementSubdomainModifier::initialize()
{
  // Clear moved elements from last execution
  _moved_elems.clear();
}

void
ElementSubdomainModifier::execute()
{
  // Compute the desired subdomain ID for the current element.
  SubdomainID subdomain_id = computeSubdomainID();

  // Don't do anything if subdomain ID is invalid
  if (subdomain_id == Moose::INVALID_BLOCK_ID)
    return;

  // If the current element's subdomain ID isn't what we want
  if (_current_elem->subdomain_id() != subdomain_id)
    _moved_elems[_current_elem->id()] = {_current_elem->subdomain_id(), subdomain_id};
}

void
ElementSubdomainModifier::threadJoin(const UserObject & in_uo)
{
  // Join the data from uo into _this_ object:
  const auto & uo = static_cast<const ElementSubdomainModifier &>(in_uo);

  _moved_elems.insert(uo._moved_elems.begin(), uo._moved_elems.end());
}

void
ElementSubdomainModifier::finalize()
{
  ElementSubdomainModifierBase::modify(_moved_elems);
}
