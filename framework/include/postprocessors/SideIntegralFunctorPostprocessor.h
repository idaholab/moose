//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"
#include "FaceArgInterface.h"

/**
 * This postprocessor computes a surface integral of the specified functor
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral() or computeFaceInfoIntegral()
 */
template <bool is_ad>
class SideIntegralFunctorPostprocessorTempl : public SideIntegralPostprocessor,
                                              public FaceArgProducerInterface
{
public:
  static InputParameters validParams();

  SideIntegralFunctorPostprocessorTempl(const InputParameters & parameters);

  bool hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const override;

protected:
  /**
   * Compute contribution from an element face, either on a boundary or between two active elements
   * @param fi the FaceInfo, containing the geometric information of the face
   * @return the integral for this element (_current_elem) and side (_current_side)
   */
  virtual Real computeFaceInfoIntegral(const FaceInfo * fi) override;

  Real computeQpIntegral() override;

  /// Check if the functor and the prefactor are defined on the primary block by the sideset
  bool checkFunctorDefinedOnSideBlock() const;

  /// Error with a helpful message if the functor is not defined on the primary block by the sideset
  void errorFunctorNotDefinedOnSideBlock() const;

  /// Functor being integrated
  const Moose::Functor<GenericReal<is_ad>> & _functor;

  /// Factor multiplying the functor being integrated
  const Moose::Functor<GenericReal<is_ad>> & _prefactor;

  /// Whether to skip integrating where the functors are not both defined
  const bool _partial_integral;

private:
  template <typename T>
  Real computeLocalContribution(const T & functor_arg) const;
};

typedef SideIntegralFunctorPostprocessorTempl<false> SideIntegralFunctorPostprocessor;
typedef SideIntegralFunctorPostprocessorTempl<true> ADSideIntegralFunctorPostprocessor;
