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
#include "ExtremeValueBase.h"

/// A postprocessor for collecting an extreme value for a functor with an element argument
template <bool is_ad>
class ElementExtremeFunctorValueTempl : public ExtremeValueBase<ElementPostprocessor>
{
public:
  static InputParameters validParams();

  ElementExtremeFunctorValueTempl(const InputParameters & parameters);
  virtual void execute() override { computeExtremeValue(); }

protected:
  virtual std::pair<Real, Real> getProxyValuePair() override;

  /// Functor to search the extrema for
  const Moose::Functor<GenericReal<is_ad>> & _functor;

  /**
   * A proxy functor used to find the quadrature point at
   * which to evaluate the functor. If not provided, defaults to the functor.
   */
  const Moose::Functor<GenericReal<is_ad>> & _proxy_functor;
};

typedef ElementExtremeFunctorValueTempl<false> ElementExtremeFunctorValue;
typedef ElementExtremeFunctorValueTempl<true> ADElementExtremeFunctorValue;
