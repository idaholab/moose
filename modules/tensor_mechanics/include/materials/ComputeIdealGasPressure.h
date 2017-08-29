/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEIDEALGASPRESSURE_H
#define COMPUTEIDEALGASPRESSURE_H

#include "ComputeGasPressureBase.h"

/**
 * Sets up an isotropic extra Stress with a trace that equals the pressure of
 * an ideal gas with the given density.
 * \f$ PV=Nk_BT \f$
 */
class ComputeIdealGasPressure : public ComputeGasPressureBase
{
public:
  ComputeIdealGasPressure(const InputParameters & parameters);

protected:
  virtual void computeQpExtraStress();
};

#endif // COMPUTEIDEALGASPRESSURE_H
