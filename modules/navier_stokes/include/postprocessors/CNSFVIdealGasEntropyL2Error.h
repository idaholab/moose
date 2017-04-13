/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVIDEALGASENTROPYL2ERROR_H
#define CNSFVIDEALGASENTROPYL2ERROR_H

#include "ElementIntegralPostprocessor.h"
#include "SinglePhaseFluidProperties.h"

// Forward Declarations
class CNSFVIdealGasEntropyL2Error;

template <>
InputParameters validParams<CNSFVIdealGasEntropyL2Error>();

/**
 * A PostProcessor object to calculate the L2 error of ideal gas entropy production for the CNS
 * equations
 */
class CNSFVIdealGasEntropyL2Error : public ElementIntegralPostprocessor
{
public:
  CNSFVIdealGasEntropyL2Error(const InputParameters & parameters);

  /**
   * Get the ideal gas entropy L2 error.
   */
  virtual Real getValue();

protected:
  virtual Real computeQpIntegral();

  const SinglePhaseFluidProperties & _fp;

  Real _inf_rho;
  Real _inf_pres;

  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> & _pres;
};

#endif
