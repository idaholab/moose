/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef PORFLOWMATERIALPOROSITYCONST_H
#define PORFLOWMATERIALPOROSITYCONST_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorFlowVarNames.h"

//Forward Declarations
class PorFlowMaterialPorosityConst;

template<>
InputParameters validParams<PorFlowMaterialPorosityConst>();

/**
 * Material designed to provide the porosity
 * which is assumed constant
 */
class PorFlowMaterialPorosityConst : public DerivativeMaterialInterface<Material>
{
public:
  PorFlowMaterialPorosityConst(const InputParameters & parameters);

protected:

  /// constant value of porosity
  const Real _input_porosity;

  /// The variable names UserObject for the Porous-Flow variables
  const PorFlowVarNames & _porflow_name_UO;

  /// porosity
  MaterialProperty<Real> & _porosity;

  /// old value of porosity (which is, of course = _porosity in this case)
  MaterialProperty<Real> & _porosity_old;

  /// d(porosity)/d(PorFlow variable)
  MaterialProperty<std::vector<Real> > & _dporosity_dvar;

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
};

#endif //PORFLOWMATERIALPOROSITYCONST_H
