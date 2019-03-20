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
  return MooseEnum("average jump_master_minus_slave jump_slave_minus_master "
                   "jump_abs master slave",
                   "average");
}

Real
getQuantity(const MooseEnum interface_value_type, const Real value_master, const Real value_slave)
{
  Real result = 0.;

  switch (interface_value_type)
  {
    case 0: /*average*/
      result = (value_master + value_slave) * 0.5;
      break;
    case 1: /*jump_master_minus_slave*/
      result = (value_master - value_slave);
      break;
    case 2: /*jump_slave_minus_master*/
      result = (value_slave - value_master);
      break;
    case 3: /*jump_abs*/
      result = std::abs(value_slave - value_master);
      break;
    case 4: /*master*/
      result = value_master;
      break;
    case 5: /*slave*/
      result = value_slave;
      break;
    default:
      mooseError("InterfaceIntegralMaterialPropertyPostprocessor: the supplied integral "
                 "type is not in the list. Available options are: ",
                 InterfaceAverageOptions().getRawNames());
  }
  return result;
}
}
