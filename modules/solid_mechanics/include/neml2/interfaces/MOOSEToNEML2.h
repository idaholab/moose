//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NEML2Utils.h"
#include "InputParameters.h"

#ifdef NEML2_ENABLED
#include "neml2/models/Model.h"
#endif

/**
 * Common interface for inserting gathered MOOSE data into the NEML2 material model.
 *
 * This interface handles the insertion into both NEML2 input variable and NEML2 model parameter.
 *
 * Users are only required to provide the name of the variable/parameter, and we perform a run-time
 * introspection of the NEML2 model to determine if the supplied name is for a NEML2 variable or for
 * a NEML2 model parameter.
 */
class MOOSEToNEML2
{
public:
  static InputParameters validParams();

  MOOSEToNEML2(const InputParameters & params);

#ifdef NEML2_ENABLED
  enum class Mode
  {
    VARIABLE,
    OLD_VARIABLE,
    PARAMETER,
    UNDEFINED
  };

  /**
   * Change the mode of operation
   *
   * The NEML2ModelExecutor user object performs run-time introspection of the NEML2 model to
   * determine if the supplied name is for a NEML2 variable or for a NEML2 model parameter.
   * It then uses this method to change the mode of operation of the MOOSEToNEML2 gatherer.
   */
  void setMode(Mode) const;

  /// Get the current mode of operation
  Mode getMode() const { return _mode; }

  /// Perform error checking after _mode has been set
  virtual void checkMode() const;

  /// Raw name of the NEML2 variable/parameter
  const std::string & NEML2Name() const { return _raw_name; }

  /// Name of the NEML2 input variable (only meaningful when _mode == VARIABLE)
  const neml2::VariableName & NEML2VariableName() const;

  /// Name of the NEML2 parameter (only meaningful when _mode == PARAMETER)
  const std::string & NEML2ParameterName() const;

  /// Convert data gathered from MOOSE into neml2::Tensor
  virtual neml2::Tensor gatheredData() const = 0;

  /// Insert the gathered data into the NEML2 material model
  void insertInto(neml2::ValueMap &, std::map<std::string, neml2::Tensor> &) const;

protected:
  /// Whether we should insert into NEML2 input variable or NEML2 model parameter
  mutable Mode _mode;

  /// NEML2 input variable to transfer data to
  mutable neml2::VariableName _neml2_variable;

  /// NEML2 parameter to transfer data to
  mutable std::string _neml2_parameter;

private:
  /// Raw name of the input variable or model parameter
  const std::string _raw_name;
#endif
};
