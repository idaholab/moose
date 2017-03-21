/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MATERIALVECTORPOSTPROCESSOR_H
#define MATERIALVECTORPOSTPROCESSOR_H

#include "ElementVectorPostprocessor.h"

class MaterialVectorPostprocessor;

template <>
InputParameters validParams<MaterialVectorPostprocessor>();

/// This postprocessor records all scalar material properties of the specified
/// material object on specified elements at the indicated execution points
/// (e.g. initial, timestep_begin, etc.).  Non-scalar properties are ignored
/// with a warning.
class MaterialVectorPostprocessor : public ElementVectorPostprocessor
{
public:
  MaterialVectorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

private:
  /// Sorts all data in the VectorPostProcessorValue objects so that output
  /// from this postprocessor is ordered consistently across arbitrary number
  /// of parallel jobs.
  void sortVecs();

  /// Element ids to record material properties for.
  std::set<unsigned int> _elem_filter;

  /// Column of element id info.
  VectorPostprocessorValue & _elem_ids;

  /// Column of quadrature point indices.
  VectorPostprocessorValue & _qp_ids;

  /// Columns for each (scalar) property of the material.
  std::vector<VectorPostprocessorValue *> _prop_vecs;

  /// Reference to each material property - used to retrieve the actual
  /// property values at every execution point.
  std::vector<const PropertyValue *> _prop_refs;

  /// Names for every property in the material - used for determining if
  /// properties are scalar or not.
  std::vector<std::string> _prop_names;
};

#endif
