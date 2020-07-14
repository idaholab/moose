#pragma once

#include "AdvectiveFluxKernel.h"

class CNSMassAdvectiveFlux;

declareADValidParams(CNSMassAdvectiveFlux);

/**
 * Kernel representing the advective component of the conservation of mass
 * equation, with strong form $\nabla\cdot\left(\epsilon\rho_f\vec{V}\right)$.
 */

class CNSMassAdvectiveFlux : public AdvectiveFluxKernel
{
public:
  CNSMassAdvectiveFlux(const InputParameters & parameters);

protected:
  virtual ADReal advectedField() override;

  virtual ADReal strongResidual() override;


};
