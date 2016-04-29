/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWFLUIDMASS_H
#define POROUSFLOWFLUIDMASS_H

#include "ElementIntegralVariablePostprocessor.h"
#include "PorousFlowDictator.h"

// Forward Declarations
class PorousFlowFluidMass;

template<>
InputParameters validParams<PorousFlowFluidMass>();

/**
 * Postprocessor produces the mass of a given fluid component in a region
 */
class PorousFlowFluidMass: public ElementIntegralVariablePostprocessor
{
public:
  PorousFlowFluidMass(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

  unsigned int _component_index;

  /// holds info on the PorousFlow variables
  const PorousFlowDictator & _dictator_UO;

  /// porosity
  const MaterialProperty<Real> & _porosity;

  /// fluid density of each phase
  const MaterialProperty<std::vector<Real> > & _fluid_density;

  /// fluid saturation of each phase
  const MaterialProperty<std::vector<Real> > & _fluid_saturation;

  /// fluid mass-fraction matrix
  const MaterialProperty<std::vector<std::vector<Real> > > & _mass_frac;
};

#endif //POROUSFLOWFLUIDMASS_H
