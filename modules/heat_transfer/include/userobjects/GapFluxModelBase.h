//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ModularGapConductanceConstraint.h"
#include "InterfaceUserObjectBase.h"
#include "ADFunctorInterface.h"

/**
 * Base class for gap flux models used by ModularGapConductanceConstraint
 */
class GapFluxModelBase : public InterfaceUserObjectBase, public ADFunctorInterface
{
public:
  static InputParameters validParams();

  GapFluxModelBase(const InputParameters & parameters);

  /**
   * Cache geometry-related information from the mortar constraint
   */
  virtual ADReal
  computeFluxInternal(const ModularGapConductanceConstraint & mortar_constraint) const;

  /**
   * Compute gap physics used cache information in GapFluxModelBase
   */
  virtual ADReal computeFlux() const = 0;

  virtual void finalize() final{};
  virtual void threadJoin(const UserObject &) final{};

protected:
  mutable unsigned int _qp;
  mutable ADReal _gap_width;
  mutable ADReal _surface_integration_factor;
  mutable ADReal _adjusted_length;
  mutable ADReal _normal_pressure;

  /// The secondary quadrature point location
  mutable Moose::ElemPointArg _secondary_point;
  /// The primary quadrature point location
  mutable Moose::ElemPointArg _primary_point;
};
