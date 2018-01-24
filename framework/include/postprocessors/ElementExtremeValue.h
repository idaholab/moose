//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENTEXTREMEVALUE_H
#define ELEMENTEXTREMEVALUE_H

#include "ElementVariablePostprocessor.h"

// Forward Declarations
class ElementExtremeValue;

// Input parameters
template <>
InputParameters validParams<ElementExtremeValue>();

/// A postprocessor for collecting the nodal min or max value
class ElementExtremeValue : public ElementVariablePostprocessor
{
public:
  /// Type of extreme value we are going to compute
  enum ExtremeType
  {
    MAX,
    MIN
  };

  /**
   * Class constructor
   * @param parameters The input parameters
   */
  ElementExtremeValue(const InputParameters & parameters);

  virtual void initialize() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// Get the extreme value at each quadrature point
  virtual void computeQpValue() override;

  /// The extreme value type
  ExtremeType _type;

  /// The extreme value
  Real _value;
};

#endif
