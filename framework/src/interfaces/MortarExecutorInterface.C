//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarExecutorInterface.h"
#include "AutomaticMortarGeneration.h"

bool
MortarExecutorInterface::mortarMaterialsNeedSetup(const AutomaticMortarGeneration & amg) const
{
  // We use .count() here instead of libmesh_map_find because this is a coverage check, not a
  // lookup: a missing key is the expected "needs setup" outcome, not an error to throw on.
  auto keysCover = [](const std::map<SubdomainID, std::deque<MaterialBase *>> & mats,
                      const std::set<SubdomainID> & sub_ids)
  {
    for (const auto sub_id : sub_ids)
      if (!mats.count(sub_id))
        return false;
    return true;
  };

  return !keysCover(_secondary_ip_sub_to_mats, amg.secondaryIPSubIDs()) ||
         !keysCover(_primary_ip_sub_to_mats, amg.primaryIPSubIDs());
}
