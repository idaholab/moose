//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALEXTREMEVALUE_H
#define NODALEXTREMEVALUE_H

#include "NodalVariablePostprocessor.h"

// Forward Declarations
class NodalExtremeValue;

// Input parameters
template <>
InputParameters validParams<NodalExtremeValue>();

/// A postprocessor for collecting the nodal min or max value
class NodalExtremeValue : public NodalVariablePostprocessor
{
public:
  /// What type of extreme value we are going to compute
  enum ExtremeType
  {
    MAX,
    MIN
  };

  /**
   * Class constructor
   * @param parameters The input parameters
   */
  NodalExtremeValue(const InputParameters & parameters);
  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// The extreme value type ("min" or "max")
  ExtremeType _type;

  /// The extreme value
  Real _value;
};

#endif
