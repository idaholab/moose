//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppTransfer.h"
#include "ReporterInterface.h"

class ControlsReceiver;
<<<<<<< HEAD
/**
 * Copies optimization data to a ControlsReceiver object.
 */
=======
    /**
     * Copies optimization data to a ControlsReceiver object.
     */
>>>>>>> f85813ccc5 (addressing new review comments  (#21885))
class OptimizationParameterTransfer : public MultiAppTransfer, public ReporterInterface
{
public:
  static InputParameters validParams();

  OptimizationParameterTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void execute() override;

private:
  /**
   * Return the ControlsReceiver object and perform error checking.
   * @param app_index The global sup-app index
   */
  ControlsReceiver * getReceiver(unsigned int app_index);

  /// Value names from OptimizationReporter
  const std::vector<ReporterValueName> & _value_names;
  /// Parameter names in sub-application
  const std::vector<std::string> & _parameters;
  /// The name of the ControlsReceiver Control object on the sub-application
  const std::string & _receiver_name;

  /// Parameter values from OptimizationReporter
  std::vector<const std::vector<Real> *> _values;
};
