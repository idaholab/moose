/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GUARANTEE_H
#define GUARANTEE_H

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

#endif // GUARANTEE_H
