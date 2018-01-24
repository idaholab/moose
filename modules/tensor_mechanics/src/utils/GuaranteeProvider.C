/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
