//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"
#include "libmesh/communicator.h"
#include "Coupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "ComputeBlockOrientationBase.h"

// Forward Declarations
class MooseMesh;

class BlockOrientationFromUserObject : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  BlockOrientationFromUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// Reference to the mesh
  MooseMesh & _mesh;

  /// User object to grab average value from
  const UserObjectName & _uo_name;
  const ComputeBlockOrientationBase * _uo;

  /// Number of columns, representing the number of features in the VectorPostprocessor
  int _num_cols;

  /// Number of rows, representing the number of data entries in the VectorPostprocessor
  int _num_rows;

  /// Vector of outputs, where each entry is the vector of average values for single variable in each block
  std::vector<VectorPostprocessorValue *> _output_vector;
};
