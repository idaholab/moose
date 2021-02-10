//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AdvectiveFluxKernel.h"

class CNSFluidEnergyAdvectiveFlux;

declareADValidParams(CNSFluidEnergyAdvectiveFlux);

/**
 * Kernel representing the advective component of the conservation of fluid energy
 * equation, with strong form $\nabla\cdot\left(\epsilon\rho_fH_f\vec{V}\right)$.
 */
class CNSFluidEnergyAdvectiveFlux : public AdvectiveFluxKernel
{
public:
  CNSFluidEnergyAdvectiveFlux(const InputParameters & parameters);

protected:
  virtual ADReal advectedField() override;

  virtual ADReal strongResidual() override;

  /// specific total enthalpy
  const ADMaterialProperty<Real> & _specific_total_enthalpy;

  /// momentum
  const ADMaterialProperty<RealVectorValue> & _momentum;

  /// total energy gradient
  const ADMaterialProperty<RealVectorValue> & _grad_rho_et;

  /// pressure gradient
  const ADMaterialProperty<RealVectorValue> & _grad_pressure;

};
