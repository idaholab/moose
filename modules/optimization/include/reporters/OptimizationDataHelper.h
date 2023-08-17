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

class InputParameters;

class OptimizationDataHelper : public MooseObject, public TransientInterface, public Reporter
{
public:
  static InputParameters validParams();
  OptimizationDataHelper(const InputParameters & parameters);
  void computeMisfit();

  std::vector<Real> & getMeasurementValues() { return _measurement_values; }
  std::vector<Real> & getSimValues() { return _simulation_values; }
  std::vector<Real> & getMisfitValues() { return _misfit_values; }

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
  /// private method for testing optimizationData with test src
  void setSimulationValuesForTesting(std::vector<Real> & data);
  /// variable
  std::vector<MooseVariableFieldBase *> _var_vec;
  /// Weight names to reporter values
  std::vector<std::vector<Real> *> _variable_weights;
  /// Weight names to reporter values map created from input file
  std::map<std::string, std::vector<Real> *> _weight_names_weights_map;
  /// helper to check data sizes
  void errorCheckDataSize();
  /// Reference to the FEProblemBase for this helper object
  FEProblemBase & _fe_problem;
  /// Thread ID of this helper object
  const THREAD_ID _tid;
};
