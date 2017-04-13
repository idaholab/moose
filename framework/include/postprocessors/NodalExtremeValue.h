/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
