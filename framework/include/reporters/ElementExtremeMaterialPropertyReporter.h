//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementReporter.h"

/// Determines the location of the extreme (minimum or maximum) value
/// of a material property, as well as values of a specified set of
/// properties at that location.
template <bool is_ad>
class ElementExtremeMaterialPropertyReporterTempl : public ElementReporter
{
public:
  static InputParameters validParams();

  /// Type of extreme value to compute
  enum class ExtremeType
  {
    MAX,
    MIN
  };

  ElementExtremeMaterialPropertyReporterTempl(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & uo) override;

protected:
  /**
   * Check a given quadrature point for the extreme value, and capture
   * the extreme value as well as the other associated data in member
   * variables
   */
  void computeQpValue();

  /// Material property for which to find extreme
  const GenericMaterialProperty<Real, is_ad> & _mat_prop;

  /// Type of extreme value to compute
  const ExtremeType _type;

  /// Extreme value
  Real & _extreme_value;

  /// Coordinates of point with extreme value
  Point & _coordinates;

  /// Pointers to other reported material property objects
  std::vector<const GenericMaterialProperty<Real, is_ad> *> _additional_reported_properties;

  /// Values of other reported material properties
  std::vector<Real *> _additional_reported_property_values;

  /// Current quadrature point
  unsigned int _qp;
};

typedef ElementExtremeMaterialPropertyReporterTempl<false> ElementExtremeMaterialPropertyReporter;
typedef ElementExtremeMaterialPropertyReporterTempl<true> ADElementExtremeMaterialPropertyReporter;
