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
class PorousFlowFluidMass: public ElementIntegralPostprocessor
{
public:
  PorousFlowFluidMass(const InputParameters & parameters);

protected:
  virtual Real computeIntegral() override;
  virtual Real computeQpIntegral() override;

  /// Holds info on the PorousFlow variables
  const PorousFlowDictator & _dictator;
  /// The fluid component index that this Postprocessor applies to
  const unsigned int _fluid_component;
  /// The phase indices that this Postprocessor is restricted to
  std::vector<unsigned int> _phase_index;
  /// Porosity
  const MaterialProperty<Real> & _porosity;
  /// Phase density (kg/m^3)
  const MaterialProperty<std::vector<Real> > & _fluid_density;
  /// Phase saturation (-)
  const MaterialProperty<std::vector<Real> > & _fluid_saturation;
  /// Mass fraction of each fluid component in each phase
  const MaterialProperty<std::vector<std::vector<Real> > > & _mass_fraction;
  /// Saturation threshold - only fluid mass at saturations below this are calculated
  const Real _saturation_threshold;
  /// the variable for the corresponding PorousFlowMassTimeDerivative Kernel: this provides test functions
  MooseVariable * const _var;
};

#endif //POROUSFLOWFLUIDMASS_H
