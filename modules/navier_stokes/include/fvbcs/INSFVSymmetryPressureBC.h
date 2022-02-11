//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"
#include "INSFVSymmetryBC.h"

class InputParameters;

/**
 * A symmetry boundary condition for the pressure variable
 */
class INSFVSymmetryPressureBC : public FVFluxBC, public INSFVSymmetryBC
{
public:
  static InputParameters validParams();
  INSFVSymmetryPressureBC(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;
};

typedef INSFVSymmetryPressureBC INSFVSymmetryScalarBC;
