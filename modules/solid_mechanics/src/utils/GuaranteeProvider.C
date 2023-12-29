//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GuaranteeProvider.h"
#include "MooseObject.h"

GuaranteeProvider::GuaranteeProvider(const MooseObject * /*moose_object*/) {}

bool
GuaranteeProvider::hasGuarantee(const MaterialPropertyName & prop_name, Guarantee guarantee)
{
  auto it = _guarantees.find(prop_name);
  if (it == _guarantees.end())
    return false;

  auto it2 = it->second.find(guarantee);
  return it2 != it->second.end();
}

void
GuaranteeProvider::issueGuarantee(const MaterialPropertyName & prop_name, Guarantee guarantee)
{
  // intentional insertion
  _guarantees[prop_name].insert(guarantee);
}

void
GuaranteeProvider::revokeGuarantee(const MaterialPropertyName & prop_name, Guarantee guarantee)
{
  auto it = _guarantees.find(prop_name);
  if (it != _guarantees.end())
    it->second.erase(guarantee);
}
