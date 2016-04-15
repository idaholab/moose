/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef PORFLOWMATERIALRELATIVEPERMEABILITYCOREY_H
#define PORFLOWMATERIALRELATIVEPERMEABILITYCOREY_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowMaterialRelativePermeabilityCorey;

template<>
InputParameters validParams<PorousFlowMaterialRelativePermeabilityCorey>();

/**
 * Material designed to calculate Corey-type relative permeability of an arbitrary phase given the saturation and 
 * Corey exponent of that phase.
 */
class PorousFlowMaterialRelativePermeabilityCorey : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterialRelativePermeabilityCorey(const InputParameters & parameters);

protected:

  /// Core exponent n_j for phase Sj
  const Real _n;

  /// phase number of fluid that we're dealing with
  const unsigned int _phase_num;

  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _porflow_name_UO;

  /// nodal Saturation of each phase
  const MaterialProperty<std::vector<Real> > & _saturation;

  /// d(nodal saturation)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real> > > & _dsaturation_dvar;

  /// computed nodal relative permeability of phase j
  MaterialProperty<Real> & _relative_permeability;

  /// d(nodal relative permeability)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real> > & _drelative_permeability_dvar;

  virtual void computeQpProperties();
};

#endif //PORFLOWMATERIALRELATIVEPERMEABILITYCOREY_H
