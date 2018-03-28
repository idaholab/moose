//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VECTORPOSTPROCESSORTRANSFER_H
#define VECTORPOSTPROCESSORTRANSFER_H

#include "MultiAppTransfer.h"

// Forward declarations
class VectorPostprocessorTransfer;

template <>
InputParameters validParams<VectorPostprocessorTransfer>();

/**
 * Copies the value of a vector postprocessor from the Multiapp to a aux variable in the master.
 */
class VectorPostprocessorTransfer : public MultiAppTransfer
{
public:
  VectorPostprocessorTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// Name of the vectorpostprocessor in the sub app
  VectorPostprocessorName _vector_postprocessor;

  /// Name of the variables in the master app to which the data from VPP is mapped to
  std::vector<AuxVariableName> _to_var_names;

  /// Names of the vectors (corresponding to variable values) that need to be extracted from VPP
  std::vector<std::string> _variable_vector_names;

  /// Boolean flag to check if the tranfer occurs on a selected set of blocks in master app
  const bool _has_blocks;

  /// Component in master app along which the variables are interpolated
  const unsigned int _master_component;

  /// Component in sub app along which the variables are interpolated
  const unsigned int _subapp_component;

  /// Position in direction other than master_component to which data from VPP is transferred to
  std::vector<Real> _horizontal_location_1;

  /// Position in direction other than master_component to which data from VPP is transferred to
  std::vector<Real> _horizontal_location_2;
};

#endif /* VECTORPOSTPROCESSORTRANSFER_H */
