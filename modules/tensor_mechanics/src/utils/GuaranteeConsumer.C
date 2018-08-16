//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GuaranteeConsumer.h"
#include "GuaranteeProvider.h"

#include "MooseObject.h"
#include "MaterialBase.h"
#include "MooseMesh.h"
#include "FEProblemBase.h"
#include "InputParameters.h"
#include "BlockRestrictable.h"

GuaranteeConsumer::GuaranteeConsumer(MooseObject * moose_object)
  : _gc_params(moose_object->parameters()),
    _gc_feproblem(_gc_params.get<FEProblemBase *>("_fe_problem_base")),
    _gc_block_restrict(dynamic_cast<BlockRestrictable *>(moose_object))
{
}

bool
GuaranteeConsumer::hasGuaranteedMaterialProperty(const MaterialPropertyName & prop_name,
                                                 Guarantee guarantee)
{
  if (!_gc_feproblem->startedInitialSetup())
    mooseError("hasGuaranteedMaterialProperty() needs to be called in initialSetup()");

  // Reference to MaterialWarehouse for testing and retrieving block ids
  const auto & warehouse = _gc_feproblem->getMaterialWarehouse();

  // Complete set of ids that this object is active
  const auto & ids = (_gc_block_restrict && _gc_block_restrict->blockRestricted())
                         ? _gc_block_restrict->blockIDs()
                         : _gc_feproblem->mesh().meshSubdomains();

  // Loop over each id for this object
  for (const auto & id : ids)
  {
    // If block materials exist, look if any issue the required guarantee
    if (warehouse.hasActiveBlockObjects(id))
    {
      const std::vector<std::shared_ptr<MaterialBase>> & mats = warehouse.getActiveBlockObjects(id);
      for (const auto & mat : mats)
      {
        const auto & mat_props = mat->getSuppliedItems();
        if (mat_props.count(prop_name))
        {
          auto guarantee_mat = dynamic_cast<GuaranteeProvider *>(mat.get());
          if (guarantee_mat && !guarantee_mat->hasGuarantee(prop_name, guarantee))
          {
            // we found at least one material on the set of block we operate on
            // that does _not_ provide the requested guarantee
            return false;
          }
        }
      }
    }
  }

  return true;
}
