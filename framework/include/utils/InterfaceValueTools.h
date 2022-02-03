//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseTypes.h"

// Forward declarations
class MooseEnum;

namespace InterfaceValueTools
{
/*
 * Return the scalar_type MooseEnum
 */
MooseEnum InterfaceAverageOptions();

/*
 * Return scalar quantity across an interface based on the user specified
 * _interface_value_type. First parameter is the average type, the second the value
 * on the primary surface, and the third parameter is the value on secondary
 * surface of the interface
 */
Real getQuantity(const MooseEnum /*interface_value_type*/,
                 const Real /*value_primary*/,
                 const Real /*value_secondary*/);
}
