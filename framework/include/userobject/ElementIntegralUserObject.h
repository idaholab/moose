//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENTINTEGRALUSEROBJECT_H
#define ELEMENTINTEGRALUSEROBJECT_H

// MOOSE includes
#include "ElementUserObject.h"

// Forward Declarations
class ElementIntegralUserObject;

template <>
InputParameters validParams<ElementIntegralUserObject>();

/**
 * This postprocessor computes a volume integral of the specified
 * variable.
 *
 * Note that specializations of this integral are possible by deriving
 * from this class and overriding computeQpIntegral().
 */
class ElementIntegralUserObject : public ElementUserObject
{
public:
  ElementIntegralUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override {}

  /// Returns the integral value
  virtual Real getValue();

protected:
  virtual Real computeQpIntegral() = 0;
  virtual Real computeIntegral();

  unsigned int _qp;

  Real _integral_value;
};

#endif
