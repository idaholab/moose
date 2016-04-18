/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWMATERIALVISCOSITYCONST_H
#define POROUSFLOWMATERIALVISCOSITYCONST_H

#include "PorousFlowMaterialFluidPropertiesBase.h"

class PorousFlowMaterialViscosityConst;

template<>
InputParameters validParams<PorousFlowMaterialViscosityConst>();

/**
 * Material designed to provide the viscosity
 * which is assumed constant
 */
class PorousFlowMaterialViscosityConst : public PorousFlowMaterialFluidPropertiesBase
{
public:
  PorousFlowMaterialViscosityConst(const InputParameters & parameters);

protected:

  /// constant value of viscosity
  const Real _input_viscosity;

  /// the phase number
  unsigned int _phase_num;

  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _porflow_name_UO;

  /// viscosity
  MaterialProperty<Real> & _viscosity;

  virtual void computeQpProperties();
};

#endif //POROUSFLOWMATERIALVISCOSITYCONST_H
