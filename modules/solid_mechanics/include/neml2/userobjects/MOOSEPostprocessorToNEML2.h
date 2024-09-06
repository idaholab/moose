//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MOOSEToNEML2Unbatched.h"

/**
 * Gather a MOOSE postprocessor value for insertion into the NEML2 model.
 */
template <unsigned int state>
class MOOSEPostprocessorToNEML2Templ : public MOOSEToNEML2Unbatched
{
public:
  static InputParameters validParams();

  MOOSEPostprocessorToNEML2Templ(const InputParameters & params);

#ifdef NEML2_ENABLED
  neml2::Tensor gatheredData() const override;

protected:
  /// Coupled MOOSE postprocessor value to read data from
  const Real & _moose_pp;
#endif
};

using MOOSEPostprocessorToNEML2 = MOOSEPostprocessorToNEML2Templ<0>;
using MOOSEOldPostprocessorToNEML2 = MOOSEPostprocessorToNEML2Templ<1>;
