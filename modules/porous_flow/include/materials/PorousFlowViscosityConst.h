/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWVISCOSITYCONST_H
#define POROUSFLOWVISCOSITYCONST_H

#include "PorousFlowFluidPropertiesBase.h"

class PorousFlowViscosityConst;

template <>
InputParameters validParams<PorousFlowViscosityConst>();

/**
 * Material that provides a constant viscosity
 */
class PorousFlowViscosityConst : public PorousFlowFluidPropertiesBase
{
public:
  PorousFlowViscosityConst(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// constant value of viscosity
  const Real _input_viscosity;

  /// viscosity
  MaterialProperty<Real> & _viscosity;
};

#endif // POROUSFLOWVISCOSITYCONST_H
