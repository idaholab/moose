//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADScalarKernel.h"
#include "ADFunctorInterface.h"

class SinglePhaseFluidProperties;

class ADIncompressibleMomentumSPBase : public ADScalarKernel, public ADFunctorInterface
{
  using Base = ADScalarKernel;

public:
  ADIncompressibleMomentumSPBase(const InputParameters & parameters);
  static InputParameters validParams();
  virtual void initialSetup() override;

protected:
  virtual ADReal computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual ADReal computeFrictionFactor(const ADReal & mu,
                                       const ADReal & G,
                                       const ADReal & Dh,
                                       const unsigned int j);
  virtual ADReal massFlowRate() const = 0;
  virtual ADReal pressureDrop() const = 0;
  /// Spatial argument used for every functor evaluation
  Moose::ElemPointArg functorArg() const { return {_functor_elem, _functor_point, false}; }

  /// Number of coupled temperature variables
  const size_t _n_temps;
  /// Coupled temperature variables
  std::vector<const ADVariableValue *> _T;
  /// Property state integration flag
  const bool _is_implicit;
  /// System reference pressure
  const Moose::Functor<ADReal> & _Pref;
  /// Fluid properties object
  const SinglePhaseFluidProperties & _fp;
  /// Number of geometrically/thermally unique segments
  const size_t _n_segments;
  /// Flow area of each segment
  std::vector<const Moose::Functor<ADReal> *> _areas;
  /// Wetted perimeter of each segment
  std::vector<const Moose::Functor<ADReal> *> _perimeters;
  /// Length of each segment
  std::vector<const Moose::Functor<ADReal> *> _lengths;
  /// Height change inlet to outlet of each segment
  std::vector<const Moose::Functor<ADReal> *> _dHs;
  /// Forms loss coefficients of each segment
  std::vector<const Moose::Functor<ADReal> *> _forms_losses;
  /// Pump pressure gains of each segment
  std::vector<const Moose::Functor<ADReal> *> _dPps;
  /// Wall roughness of each segment
  std::vector<const Moose::Functor<ADReal> *> _roughnesses;
  /// Gravitational acceleration in the vertical downward direction
  const Moose::Functor<ADReal> & _gravity;
  /// Point at which spatially varying functors are evaluated
  const Point _functor_point;
  /// Element containing _functor_point (nullptr if outside the mesh / not on this process)
  const Elem * _functor_elem = nullptr;
};
