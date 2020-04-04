//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// STL includes
#include <map>

// MOOSE includes
#include "MooseTypes.h"
#include "Guarantee.h"

class MooseObject;

/**
 * Add-on class that provides the functionality to issue guarantees for
 * declared material properties. The types of guarantees are listed in Guarantees.h
 */
class GuaranteeProvider
{
public:
  GuaranteeProvider(const MooseObject * moose_object);

  bool hasGuarantee(const MaterialPropertyName & prop_name, Guarantee guarantee);

protected:
  void issueGuarantee(const MaterialPropertyName & prop_name, Guarantee guarantee);
  void revokeGuarantee(const MaterialPropertyName & prop_name, Guarantee guarantee);

private:
  std::map<MaterialPropertyName, std::set<Guarantee>> _guarantees;
};
