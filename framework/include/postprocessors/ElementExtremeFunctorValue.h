//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementPostprocessor.h"

// Forward Declarations
template <bool>
class ElementExtremeFunctorValueTempl;
typedef ElementExtremeFunctorValueTempl<false> ElementExtremeFunctorValue;
typedef ElementExtremeFunctorValueTempl<true> ADElementExtremeFunctorValue;

/// A postprocessor for collecting an extreme value for a functor with an element argument
template <bool is_ad>
class ElementExtremeFunctorValueTempl : public ElementPostprocessor
{
public:
  static InputParameters validParams();

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
  ElementExtremeFunctorValueTempl(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override { computeValue(); }
  virtual Real getValue() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// Get the extreme value with a functor element argument
  virtual void computeValue();

  /// The extreme value type
  ExtremeType _type;

  /// Functor to search the extrema for
  const Moose::Functor<GenericReal<is_ad>> & _functor;

  /**
   * A proxy functor used to find the quadrature point at
   * which to evaluate the functor. If not provided, defaults to the functor.
   */
  const Moose::Functor<GenericReal<is_ad>> & _proxy_functor;

  /// Extreme value of the value and proxy functor at the same point
  std::pair<Real, Real> _proxy_value;
};
