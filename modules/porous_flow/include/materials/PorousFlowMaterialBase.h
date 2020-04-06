//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterial.h"
#include "DerivativeMaterialInterface.h"

/**
 * Base class for all PorousFlow materials that provide phase-dependent properties.
 * These include: fluid properties, relative permeabilities and capillary pressures.
 * and relative permeability classes. This base class checks that the specified fluid
 * phase index is valid, and provides a stringified version of the phase index to use
 * in the material property names.
 */
class PorousFlowMaterialBase : public DerivativeMaterialInterface<PorousFlowMaterial>
{
public:
  static InputParameters validParams();

  PorousFlowMaterialBase(const InputParameters & parameters);

protected:
  /// Phase number of fluid
  const unsigned int _phase_num;

  /// Stringified fluid phase number
  const std::string _phase;
};
