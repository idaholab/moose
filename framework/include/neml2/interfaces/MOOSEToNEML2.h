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

// NEML2 v3 exchanges plain at::Tensor across its C++ API; at::Tensor is available via
// NEML2Utils.h (which includes ATen) when NEML2 is enabled.

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

  /// Convert data gathered from MOOSE into an at::Tensor (shape (batch, *base_shape))
  virtual at::Tensor gatheredData() const = 0;

  /// Insert the gathered data into the NEML2 model input map
  void insertInto(std::map<std::string, at::Tensor> &) const;

private:
  /// Name of the input variable or model parameter
  const std::string _neml2_name;
#endif
};
