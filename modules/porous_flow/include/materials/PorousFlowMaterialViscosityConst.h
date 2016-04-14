/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWMATERIALVISCOSITYCONST_H
#define POROUSFLOWMATERIALVISCOSITYCONST_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowMaterialViscosityConst;

template<>
InputParameters validParams<PorousFlowMaterialViscosityConst>();

/**
 * Material designed to provide the viscosity
 * which is assumed constant
 */
class PorousFlowMaterialViscosityConst : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterialViscosityConst(const InputParameters & parameters);

protected:

  /// constant value of viscosity
  const Real _input_viscosity;

  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _porflow_name_UO;

  /// viscosity
  MaterialProperty<Real> & _viscosity;

  /// old value of viscosity (which is, of course = _viscosity in this case)
  MaterialProperty<Real> & _viscosity_old;

  /// d(viscosity)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real> > & _dviscosity_dvar;

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
};

#endif //POROUSFLOWMATERIALVISCOSITYCONST_H
