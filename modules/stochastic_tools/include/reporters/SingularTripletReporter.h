//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralReporter.h"
#include "PODMapping.h"
#include "MappingInterface.h"

/**
 * Reporter class which can print Singular Value Decompositions from PODMapping objects
 */
class SingularTripletReporter : public GeneralReporter, public MappingInterface
{
public:
  static InputParameters validParams();
  SingularTripletReporter(const InputParameters & parameters);

  void initialize() override {}
  void execute() override;
  void finalize() override {}

  void initialSetup() override;

protected:
  /// The names of the variables whose SVD should be printed
  const std::vector<VariableName> & _variable_names;

  /// Link to the PODMapping object which contains the SVDs
  PODMapping * _pod_mapping;

  ///@{
  /// Links to reporter data for left and right singular vectors together with the singular values
  std::map<VariableName, std::vector<DenseVector<Real>>> & _left_singular_vectors;
  std::map<VariableName, std::vector<DenseVector<Real>>> & _right_singular_vectors;
  std::map<VariableName, std::vector<Real>> & _singular_values;
  ///@}
};
