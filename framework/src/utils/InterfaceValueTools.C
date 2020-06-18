//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceValueTools.h"

// MOOSE includes
#include "MooseEnum.h"
#include "MooseError.h"

namespace InterfaceValueTools
{

MooseEnum
InterfaceAverageOptions()
{
  return MooseEnum("average jump_primary_minus_secondary jump_secondary_minus_primary "
                   "jump_abs primary secondary",
                   "average");
}

Real
getQuantity(const MooseEnum interface_value_type,
            const Real value_primary,
            const Real value_secondary)
{
  Real result = 0.;

  switch (interface_value_type)
  {
    case 0: /*average*/
      result = (value_primary + value_secondary) * 0.5;
      break;
    case 1: /*jump_primary_minus_secondary*/
      result = (value_primary - value_secondary);
      break;
    case 2: /*jump_secondary_minus_primary*/
      result = (value_secondary - value_primary);
      break;
    case 3: /*jump_abs*/
      result = std::abs(value_secondary - value_primary);
      break;
    case 4: /*primary*/
      result = value_primary;
      break;
    case 5: /*secondary*/
      result = value_secondary;
      break;
    default:
      mooseError("InterfaceIntegralMaterialPropertyPostprocessor: the supplied integral "
                 "type is not in the list. Available options are: ",
                 InterfaceAverageOptions().getRawNames());
  }
  return result;
}
}
