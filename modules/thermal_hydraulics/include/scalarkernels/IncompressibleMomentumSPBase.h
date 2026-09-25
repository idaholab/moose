//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ScalarKernel.h"
#include "ADScalarKernel.h"
#include "FunctorInterface.h"

class SinglePhaseFluidProperties;

template <bool is_ad>
class IncompressibleMomentumSPBaseTempl
  : public std::conditional<is_ad, ADScalarKernel, ScalarKernel>::type,
    public FunctorInterface
{
  using Base = typename std::conditional<is_ad, ADScalarKernel, ScalarKernel>::type;

public:
  IncompressibleMomentumSPBaseTempl(const InputParameters & parameters);
  virtual bool isADObject() const override { return is_ad; };
  virtual void reinit() override;
  static InputParameters validParams();

protected:
  virtual GenericReal<is_ad> computeFrictionFactor(const GenericReal<is_ad> mu,
                                                   const GenericReal<is_ad> G,
                                                   const GenericReal<is_ad> Dh,
                                                   const int j);
  /// Number of coupled temperature variables
  const size_t _n_temps;
  /// Coupled temperature variables
  std::vector<const VariableValue *> _T;
  /// Property state integration flag
  const bool _is_implicit;
  /// System reference pressure
  const Moose::Functor<GenericReal<is_ad>> & _Pref;
  /// Fluid properties object
  const SinglePhaseFluidProperties & _fp;
  /// Number of geometrically/thermally unique segments
  const size_t _n_segments;
  /// Flow area of each segment
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _areas;
  /// Wetted perimeter of each segment
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _perimeters;
  /// Length of each segment
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _lengths;
  /// Angle with respect to the horizontal of each segment
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _alphas;
  /// Forms loss coefficients of each segment
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _forms_losses;
  /// Pump pressure gains of each segment
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _dPps;
  /// Wall roughness of each segment
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _roughnesses;
  /// Gravitational acceleration in the vertical downward direction
  const Moose::Functor<GenericReal<is_ad>> & _gravity;
};
