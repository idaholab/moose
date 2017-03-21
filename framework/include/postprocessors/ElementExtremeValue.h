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
