/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SPLITCHMATH_H
#define SPLITCHMATH_H

#include "SplitCHCRes.h"

//Forward Declarations
class SplitCHMath;

template<>
InputParameters validParams<SplitCHMath>();

class SplitCHMath : public SplitCHCRes
{
public:
  SplitCHMath(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeDFDC(PFFunctionType type);
};

#endif //SPLITCHMATH_H
