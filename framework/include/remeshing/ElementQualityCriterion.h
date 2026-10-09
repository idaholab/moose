//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RemeshCriterion.h"

#include "libmesh/enum_elem_quality.h"

/**
 * Fires when the worst element quality of the mesh falls below a threshold.
 *
 * The measured quantity is the minimum over the active elements of the libMesh quality metric
 * selected by the 'quality_metric' parameter, which is reduced over the communicator before it is
 * compared to the threshold.
 */
class ElementQualityCriterion : public RemeshCriterion
{
public:
  static InputParameters validParams();

  ElementQualityCriterion(const InputParameters & parameters);

  virtual bool shouldRemesh() override;

private:
  /// The quality metric measured on every active element
  const libMesh::ElemQuality _quality_metric;

  /// The quality the worst element is not allowed to fall below
  const Real _threshold;
};
