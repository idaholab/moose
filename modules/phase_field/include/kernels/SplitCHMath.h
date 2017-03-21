/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SPLITCHMATH_H
#define SPLITCHMATH_H

#include "SplitCHCRes.h"

// Forward Declarations
class SplitCHMath;

template <>
InputParameters validParams<SplitCHMath>();

/// The couple, SplitCHMath and SplitCHWRes, splits the CH equation by replacing chemical potential with 'w'.
class SplitCHMath : public SplitCHCRes
{
public:
  SplitCHMath(const InputParameters & parameters);

protected:
  virtual Real computeDFDC(PFFunctionType type);
};

#endif // SPLITCHMATH_H
