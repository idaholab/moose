// This file is part of the MOOSE framework
// https://www.mooseframework.org
//
// All rights reserved, see COPYRIGHT for full restrictions
// https://github.com/idaholab/moose/blob/master/COPYRIGHT
//
// Licensed under LGPL 2.1, please see LICENSE for details
// https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FEBOUNDARYVALUEUSEROBJECT_H
#define FEBOUNDARYVALUEUSEROBJECT_H

// Module includes
#include "FEBoundaryBaseUserObject.h"

// Forward declarations
class FEBoundaryValueUserObject;

template <>
InputParameters validParams<FEBoundaryValueUserObject>();

/**
 * This boundary FE evaluator calculates the values
 */
class FEBoundaryValueUserObject final : public FEBoundaryBaseUserObject
{
public:
  FEBoundaryValueUserObject(const InputParameters & parameters);
};

#endif // FEBOUNDARYVALUEUSEROBJECT_H
