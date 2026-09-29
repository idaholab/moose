//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableSizeMaterialPropertiesInterface.h"
#include "MaterialWarehouse.h"

std::size_t
VariableSizeMaterialPropertiesInterface::getVectorPropertySize(
    const MaterialPropertyName & prop_name) const
{
  mooseError("getVectorPropertySize() is not implemented for the material declaring property '",
             prop_name,
             "'");
}

std::pair<std::size_t, std::size_t>
VariableSizeMaterialPropertiesInterface::getMatrixPropertySize(
    const MaterialPropertyName & prop_name) const
{
  mooseError("getMatrixPropertySize() is not implemented for the material declaring property '",
             prop_name,
             "'");
}

void
Moose::checkArrayMaterialPropertySize(const MooseObject & object,
                                      const MaterialWarehouse & warehouse,
                                      const std::set<SubdomainID> & blocks,
                                      const std::string & param_name,
                                      const std::size_t count,
                                      const bool is_matrix)
{
  const auto & prop_name = object.getParam<MaterialPropertyName>(param_name);
  for (const auto id : blocks)
  {
    if (!warehouse.hasActiveBlockObjects(id))
      continue;
    for (const auto & mat : warehouse.getActiveBlockObjects(id))
    {
      if (!mat->getSuppliedItems().count(prop_name))
        continue;
      const auto * const vsmi =
          dynamic_cast<const VariableSizeMaterialPropertiesInterface *>(mat.get());
      if (!vsmi)
        continue;

      if (is_matrix)
      {
        const auto [rows, cols] = vsmi->getMatrixPropertySize(prop_name);
        if (rows != count || cols != count)
          object.paramError(param_name,
                            "The size (",
                            rows,
                            "x",
                            cols,
                            ") of material property '",
                            prop_name,
                            "' declared by material '",
                            mat->name(),
                            "' is inconsistent with the number of components of the array "
                            "variable (",
                            count,
                            ")");
      }
      else
      {
        const auto size = vsmi->getVectorPropertySize(prop_name);
        if (size != count)
          object.paramError(param_name,
                            "The size (",
                            size,
                            ") of material property '",
                            prop_name,
                            "' declared by material '",
                            mat->name(),
                            "' is inconsistent with the number of components of the array "
                            "variable (",
                            count,
                            ")");
      }
    }
  }
}
