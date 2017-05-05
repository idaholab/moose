/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWTOTALGRAVITATIONALDENSITYBASE_H
#define POROUSFLOWTOTALGRAVITATIONALDENSITYBASE_H

#include "PorousFlowMaterialVectorBase.h"

// Forward Declarations
class PorousFlowTotalGravitationalDensityBase;

template <>
InputParameters validParams<PorousFlowTotalGravitationalDensityBase>();

/**
 * Base class Material designed to provide the density of the porous medium
 */
class PorousFlowTotalGravitationalDensityBase : public PorousFlowMaterialVectorBase
{
public:
  PorousFlowTotalGravitationalDensityBase(const InputParameters & parameters);

protected:
  /// computed density at quadpoints
  MaterialProperty<Real> & _gravdensity;

  /// d(density)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real>> & _dgravdensity_dvar;
};

#endif // POROUSFLOWTOTALGRAVITATIONALDENSITYBASE_H
