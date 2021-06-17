//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

// Forward Declarations

/**
 * Computes the Q criterion as defined by the paper
 *
 * Jeong, Jinhee & Hussain, Fazle. (1995).
 * Hussain, F.: On the identification of a vortex.
 * JFM 285, 69-94. Journal of Fluid Mechanics. 285.
 * 69 - 94. 10.1017/S0022112095000462.
 *
 * This scalar quantity aids in the identification of vortices.
 * It quantifies the local balance between shear strain rate
 * and vorticity magnitude. It vanishes at the wall. Taking
 * contours of this quantity in visualization software will
 * show the vortices in the flow in a rather pronounced manner.
 */
class INSQCriterionAux : public AuxKernel
{
public:
  static InputParameters validParams();

  INSQCriterionAux(const InputParameters & parameters);

  virtual ~INSQCriterionAux() {}

protected:
  virtual Real computeValue() override;

  // Velocity gradients
  const VectorVariableGradient & _grad_velocity;
};
