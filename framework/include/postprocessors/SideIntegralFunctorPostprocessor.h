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

// Forward Declarations
template <bool>
class SideIntegralFunctorPostprocessorTempl;
typedef SideIntegralFunctorPostprocessorTempl<false> SideIntegralFunctorPostprocessor;
typedef SideIntegralFunctorPostprocessorTempl<true> SideIntegralADFunctorPostprocessor;

/**
 * This postprocessor computes a surface integral of the specified functor
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
template <bool is_ad>
class SideIntegralFunctorPostprocessorTempl : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  SideIntegralFunctorPostprocessorTempl(const InputParameters & parameters);

protected:
  using SideIntegralPostprocessor::_qp_integration;
  /**
   * Compute contribution from an element face, either on a boundary or between two active elements
   * @param fi the FaceInfo, containing the geometric information of the face
   * @ return the integral for this element (_current_elem) and side (_current_side)
   */
  virtual Real computeFaceInfoIntegral(const FaceInfo * fi);

  /// Quadrature point integration of functors is not implemented
  Real computeQpIntegral() override
  {
    mooseError("Functor side integrals are implemented over FaceInfos not quadrature points");
  };

  /// Functor being integrated
  const Moose::Functor<GenericReal<is_ad>> & _functor;

  /// Factor multiplying the functor being integrated
  const Moose::Functor<GenericReal<is_ad>> & _prefactor;

  /// Whether to skip integrating where the functors are not both defined
  const bool _partial_integral;
};
