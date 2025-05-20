//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NumericalFlux1D.h"

/**
 * Base class for computing numerical fluxes for FlowModelGasMix.
 */
class NumericalFluxGasMixBase : public NumericalFlux1D
{
public:
  static InputParameters validParams();

  NumericalFluxGasMixBase(const InputParameters & parameters);

protected:
  virtual std::vector<ADReal> convert1DInputTo3D(const std::vector<ADReal> & U_1d) const override;
  virtual std::vector<ADReal> convert3DFluxTo1D(const std::vector<ADReal> & F_3d) const override;
  virtual void transform3DFluxDirection(std::vector<ADReal> & F_3d, Real nLR_dot_d) const override;
};
