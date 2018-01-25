//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FEBOUNDARYBASEUSEROBJECT_H
#define FEBOUNDARYBASEUSEROBJECT_H

// MOOSE includes
#include "SideIntegralVariableUserObject.h"

// Module includes
#include "FEIntegralBaseUserObject.h"

class FEBoundaryBaseUserObject;

template <>
InputParameters validParams<FEBoundaryBaseUserObject>();

/**
 * This class provides the base for generating a functional expansion on a boundary by inheriting
 * from FEIntegralBaseUserObject and providing SideIntegralVariableUserObject as the template
 * parameter
 */
class FEBoundaryBaseUserObject : public FEIntegralBaseUserObject<SideIntegralVariableUserObject>
{
public:
  FEBoundaryBaseUserObject(const InputParameters & parameters);

protected:
  // Overrides from FEIntegralBaseUserObject
  virtual Point getCentroid() const final;
  virtual Real getVolume() const final;
};

#endif // FEBOUNDARYBASEUSEROBJECT_H
