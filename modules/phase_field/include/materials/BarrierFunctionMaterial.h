/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef BARRIERFUNCTIONMATERIAL_H
#define BARRIERFUNCTIONMATERIAL_H

#include "OrderParameterFunctionMaterial.h"

// Forward Declarations
class BarrierFunctionMaterial;

template <>
InputParameters validParams<BarrierFunctionMaterial>();

/**
 * Material class to provide the double well function \f$ g(\eta) \f$ for
 * the KKS system.
 *
 * \see KKSPhaseChemicalPotential
 * \see KKSCHBulk
 */
class BarrierFunctionMaterial : public OrderParameterFunctionMaterial
{
public:
  BarrierFunctionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Polynomial order of the switching function \f$ g(\eta) \f$
  MooseEnum _g_order;

  /// zero out g contribution in the eta interval [0:1]
  bool _well_only;
};

#endif // BARRIERFUNCTIONMATERIAL_H
