/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MIXEDSWITCHINGFUNCTIONMATERIAL_H
#define MIXEDSWITCHINGFUNCTIONMATERIAL_H

#include "OrderParameterFunctionMaterial.h"

// Forward Declarations
class MixedSwitchingFunctionMaterial;

template <>
InputParameters validParams<MixedSwitchingFunctionMaterial>();

/**
 * Material class to provide the switching function \f$ h(\eta) \f$ for
 * the KKS system.
 *
 * \see KKSPhaseChemicalPotential
 * \see KKSCHBulk
 */
class MixedSwitchingFunctionMaterial : public OrderParameterFunctionMaterial
{
public:
  MixedSwitchingFunctionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Polynomial order of the switching function \f$ h(\eta) \f$
  MooseEnum _h_order;

  /// Weight parameter of mixed-type h(eta)
  Real _weight;
};

#endif // MIXEDSWITCHINGFUNCTIONMATERIAL_H
