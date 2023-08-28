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
#include "MooseTypes.h"
#include "MooseUtils.h"
#include "InputParameters.h"
#include "FEProblemBase.h"
#include "MooseError.h"
#include "MooseObject.h"
#include "Reporter.h"
#include "DelimitedFileReader.h"
#include "SystemBase.h"
#include "GeneralReporter.h"
#include "OptimizationReporterBase.h"

// Forward Declarations
template <typename T>
class OptimizationDataTempl;

typedef OptimizationDataTempl<GeneralReporter> OptimizationData;

template <typename T>
class OptimizationDataTempl : public T
{
public:
  static InputParameters validParams();
  OptimizationDataTempl(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

  void computeMisfit();

protected:
  ///@{
  /// x,y,z,t and value measurement data
  std::vector<Real> & _measurement_xcoord;
  std::vector<Real> & _measurement_ycoord;
  std::vector<Real> & _measurement_zcoord;
  std::vector<Real> & _measurement_time;
  std::vector<Real> & _measurement_values;
  ///@}
  /// simulated values at measurement xyzt
  std::vector<Real> & _simulation_values;
  /// difference between simulation and measurement values at measurement xyzt
  std::vector<Real> & _misfit_values;

private:
  /// parse measurement data from csv file
  void readMeasurementsFromFile();
  /// parse measurement data from input file
  void readMeasurementsFromInput();
  /// variable
  std::vector<MooseVariableFieldBase *> _var_vec;
  /// Weight names to reporter values
  std::vector<std::vector<Real> *> _variable_weights;
  /// Weight names to reporter values map created from input file
  std::map<std::string, std::vector<Real> *> _weight_names_weights_map;
  /// helper to check data sizes
  void errorCheckDataSize();
};
