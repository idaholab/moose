/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVENTROPYPRODUCTIONAUX_H
#define CNSFVENTROPYPRODUCTIONAUX_H

#include "AuxKernel.h"
#include "SinglePhaseFluidProperties.h"

class CNSFVEntropyProductionAux;

template <>
InputParameters validParams<CNSFVEntropyProductionAux>();

/**
 * An aux kernel for calculating entropy production
 */
class CNSFVEntropyProductionAux : public AuxKernel
{
public:
  CNSFVEntropyProductionAux(const InputParameters & parameters);
  virtual ~CNSFVEntropyProductionAux() {}

protected:
  virtual Real computeValue();

  const SinglePhaseFluidProperties & _fp;

  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> & _pres;

  Real _inf_rho;
  Real _inf_pres;
};

#endif
