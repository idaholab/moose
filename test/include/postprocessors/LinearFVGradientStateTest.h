//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "GeneralPostprocessor.h"

class LinearFVGradientReader;
template <typename>
class MooseLinearVariableFV;

/**
 * Reads a requested linear FV gradient state through the public variable and reader APIs.
 */
class LinearFVGradientStateTest : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  LinearFVGradientStateTest(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override {}
  virtual void execute() override;
  virtual PostprocessorValue getValue() const override { return _value; }

private:
  /// Linear FV variable whose gradient is tested.
  MooseLinearVariableFV<Real> * const _variable;

  /// Reader returned by the first request, retained to test setup-time storage growth.
  const LinearFVGradientReader * _reader;

  /// State selected for reading.
  const unsigned int _state;

  /// Spatial gradient component selected for reading.
  const unsigned int _component;

  /// Iteration type selected for reading.
  const MooseEnum _iteration_type;

  /// Whether to read an internal face instead of an element.
  const bool _face;

  /// Element ID selected for element reads.
  const dof_id_type _element_id;

  /// Most recently read gradient value.
  PostprocessorValue _value = 0;
};
