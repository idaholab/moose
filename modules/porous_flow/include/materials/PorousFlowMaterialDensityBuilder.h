/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef PORFLOWMATERIALDENSITYBUILDER_H
#define PORFLOWMATERIALDENSITYBUILDER_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowMaterialDensityBuilder;

template<>
InputParameters validParams<PorousFlowMaterialDensityBuilder>();

/**
 * Material designed to form a std::vector of density
 * from the individual phase densities
 */
class PorousFlowMaterialDensityBuilder : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterialDensityBuilder(const InputParameters & parameters);

protected:

  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _porflow_name_UO;

  unsigned int _num_phases;

  /// computed density of the phase
  MaterialProperty<std::vector<Real> > & _density;

  /// old value of density of the phase
  MaterialProperty<std::vector<Real> > & _density_old;

  /// d(density)/d(PorousFlow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _ddensity_dvar;

  /// density of each phase
  std::vector<const MaterialProperty<Real> *> _phase_density;

  /// d(density of each phase)/d(var)
  std::vector<const MaterialProperty<std::vector<Real> > *> _dphase_density_dvar;

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
};

#endif //PORFLOWMATERIALDENSITYBUILDER_H
