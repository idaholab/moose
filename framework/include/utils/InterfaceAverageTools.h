//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEAVERAGETOOLS_H
#define INTERFACEAVERAGETOOLS_H

// MOOSE includes
#include "MooseTypes.h"

// Forward declarations
class MooseEnum;

namespace InterfaceAverageTools
{
/*
 * Return the scalar_type MooseEnum
 */
MooseEnum InterfaceAverageOptions();

/*
 * Return scalar quantity across an interface o based on the user specified
 * _average_type. First paramter is the vaerage type, the second  the value
 * on the master surface, and the third parameter is the value on slave
 * surface of the interface
 */
Real getQuantity(const MooseEnum /*average_type*/,
                 const Real /*value_master*/,
                 const Real /*value_slave*/);
}

#endif // INTERFACEAVERAGETOOLS_H
