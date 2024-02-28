//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/**
 * Enum values for guarantees that can be demanded for material properties.
 * New items can be added to this list as new guarantees become necessary.
 */
enum class Guarantee
{
  ISOTROPIC,        // applicable to RankTwo and RankFour tensors that are always isotropic
  CONSTANT_IN_TIME, // applicable to material properties that do not change over time
  UNIFORM_IN_SPACE  // applicable to material properties that have constant values in space
};
