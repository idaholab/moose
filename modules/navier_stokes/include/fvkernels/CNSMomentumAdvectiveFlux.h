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

class CNSMomentumAdvectiveFlux;

declareADValidParams(CNSMomentumAdvectiveFlux);

/**
 * Kernel representing the advective component of the momentum equation, with
 * strong form $\nabla\cdot\left(\epsilon\rho_f\vec{V}\vec{V}+\epsilon P\textbf{I}\right)$.
 */
class CNSMomentumAdvectiveFlux : public AdvectiveFluxKernel
{
public:
  CNSMomentumAdvectiveFlux(const InputParameters & parameters);

protected:
  virtual ADReal advectedField() override;

  virtual ADReal strongResidual() override;

  /// component of momentum equation
  const unsigned int & _component;

  /// momentum
  const ADMaterialProperty<RealVectorValue> & _momentum;

};
