/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVIDEALGASTOTALENTHALPYL2ERROR_H
#define CNSFVIDEALGASTOTALENTHALPYL2ERROR_H

#include "ElementIntegralPostprocessor.h"
#include "SinglePhaseFluidProperties.h"

// Forward Declarations
class CNSFVIdealGasTotalEnthalpyL2Error;

template <>
InputParameters validParams<CNSFVIdealGasTotalEnthalpyL2Error>();

/**
 * A PostProcessor object to calculate the L2 error of ideal gas total enthalpy for the CNS
 * equations
 */
class CNSFVIdealGasTotalEnthalpyL2Error : public ElementIntegralPostprocessor
{
public:
  CNSFVIdealGasTotalEnthalpyL2Error(const InputParameters & parameters);

  /**
   * Get the ideal gas total enthalpy L2 error.
   */
  virtual Real getValue();

protected:
  virtual Real computeQpIntegral();

  const SinglePhaseFluidProperties & _fp;

  Real _inf_rho;
  Real _inf_uadv;
  Real _inf_vadv;
  Real _inf_wadv;
  Real _inf_pres;

  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> & _enth;
};

#endif
