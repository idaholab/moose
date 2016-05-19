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

template<>
InputParameters validParams<PorousFlowViscosityConst>();

/**
 * Material designed to provide the viscosity
 * which is assumed constant
 */
class PorousFlowViscosityConst : public PorousFlowFluidPropertiesBase
{
public:
  PorousFlowViscosityConst(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// constant value of viscosity
  const Real _input_viscosity;

  /// the phase number
  unsigned int _phase_num;

  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _dictator;

  /// viscosity
  MaterialProperty<Real> & _viscosity;
};

#endif //POROUSFLOWVISCOSITYCONST_H
