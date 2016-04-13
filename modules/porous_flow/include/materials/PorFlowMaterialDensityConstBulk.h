/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef PORFLOWMATERIALDENSITYCONSTBULK_H
#define PORFLOWMATERIALDENSITYCONSTBULK_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorFlowVarNames.h"

//Forward Declarations
class PorFlowMaterialDensityConstBulk;

template<>
InputParameters validParams<PorFlowMaterialDensityConstBulk>();

/**
 * Material designed to calculate fluid density
 * from porepressure, assuming constant bulk modulus
 * for the fluid.
 */
class PorFlowMaterialDensityConstBulk : public DerivativeMaterialInterface<Material>
{
public:
  PorFlowMaterialDensityConstBulk(const InputParameters & parameters);

protected:

  /// density at zero porepressure
  const Real _dens0;

  /// constant bulk modulus
  const Real _bulk;

  /// phase number of fluid that we're dealing with
  const unsigned int _phase_num;

  /// The variable names UserObject for the Porous-Flow variables
  const PorFlowVarNames & _porflow_name_UO;

  /// porepressure of each phase
  const MaterialProperty<std::vector<Real> > & _porepressure;

  /// d(porepressure)/d(PorFlow variable)
  const MaterialProperty<std::vector<std::vector<Real> > > & _dporepressure_dvar;

  /// computed density of the phase
  MaterialProperty<Real> & _density;

  /// old value of density of the phase
  MaterialProperty<Real> & _density_old;

  /// d(density)/d(PorFlow variable)
  MaterialProperty<std::vector<Real> > & _ddensity_dvar;

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
};

#endif //PORFLOWMATERIALDENSITYCONSTBULK_H
