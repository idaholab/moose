//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  /// Name of the NEML2 variable/parameter
  const std::string & NEML2Name() const { return _neml2_name; }

  /// Convert data gathered from MOOSE into neml2::Tensor
  virtual neml2::Tensor gatheredData() const = 0;

  /// Insert the gathered data into the NEML2 material model
  void insertInto(std::map<std::string, neml2::Tensor> &) const;

private:
  /// Name of the input variable or model parameter
  const std::string _neml2_name;
#endif
};
