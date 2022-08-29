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

/// Determines the minimum or maximum of a material property over a volume.
template <bool is_ad>
class ElementExtremeMaterialPropertyTempl : public ElementPostprocessor
{
public:
  static InputParameters validParams();

  /// Type of extreme value to compute
  enum ExtremeType
  {
    MAX,
    MIN
  };

  ElementExtremeMaterialPropertyTempl(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  virtual void computeQpValue();

  /// Material property for which to find extreme
  const GenericMaterialProperty<Real, is_ad> & _mat_prop;

  /// Type of extreme value to compute
  ExtremeType _type;

  /// Extreme value
  Real _value;

  /// Current quadrature point
  unsigned int _qp;
};

typedef ElementExtremeMaterialPropertyTempl<false> ElementExtremeMaterialProperty;
typedef ElementExtremeMaterialPropertyTempl<true> ADElementExtremeMaterialProperty;
