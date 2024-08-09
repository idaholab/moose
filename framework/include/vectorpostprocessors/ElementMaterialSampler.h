//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementVectorPostprocessor.h"
#include <optional>

/// This postprocessor records all scalar material properties of the specified
/// material object on specified elements at the indicated execution points
/// (e.g. initial, timestep_begin, etc.).  Non-scalar properties are ignored
/// with a warning.
class ElementMaterialSampler : public ElementVectorPostprocessor
{
public:
  static InputParameters validParams();

  ElementMaterialSampler(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

private:
  /// Sorts all data in the VectorPostProcessorValue objects so that output
  /// from this postprocessor is ordered consistently across arbitrary number
  /// of parallel jobs.
  void sortVecs();

  /// Element ids to record material properties for.
  std::optional<std::set<dof_id_type>> _elem_filter;

  /// Column of element id info.
  VectorPostprocessorValue & _elem_ids;

  /// Column of quadrature point indices.
  VectorPostprocessorValue & _qp_ids;

  /// Columns of quadrature point coordinates.
  VectorPostprocessorValue & _x_coords;
  VectorPostprocessorValue & _y_coords;
  VectorPostprocessorValue & _z_coords;

  /// Columns for each (scalar) property of the material.
  std::vector<VectorPostprocessorValue *> _prop_vecs;

  /// Reference to each material property - used to retrieve the actual
  /// property values at every execution point.
  std::vector<const PropertyValue *> _prop_refs;

  /// Names for every property in the material - used for determining if
  /// properties are scalar or not.
  std::vector<std::string> _prop_names;
};
