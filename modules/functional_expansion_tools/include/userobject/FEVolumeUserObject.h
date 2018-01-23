// This file is part of the MOOSE framework
// https://www.mooseframework.org
//
// All rights reserved, see COPYRIGHT for full restrictions
// https://github.com/idaholab/moose/blob/master/COPYRIGHT
//
// Licensed under LGPL 2.1, please see LICENSE for details
// https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FEVOLUMEUSEROBJECT_H
#define FEVOLUMEUSEROBJECT_H

// MOOSE includes
#include "ElementIntegralVariableUserObject.h"
#include "FEIntegralBaseUserObject.h"

// Forward declarations
class FEVolumeUserObject;

template <>
InputParameters validParams<FEVolumeUserObject>();

/**
 * This volumetric FE calculates the value
 */
class FEVolumeUserObject final : public FEIntegralBaseUserObject<ElementIntegralVariableUserObject>
{
public:
  FEVolumeUserObject(const InputParameters & parameters);

protected:
  // Overrides from FEIntegralBaseUserObject
  virtual Point getCentroid() const;
  virtual Real getVolume() const;
};

#endif // FEVOLUMEUSEROBJECT_H
