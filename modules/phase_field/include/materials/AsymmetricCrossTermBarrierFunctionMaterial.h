/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ASYMMETRICCROSSTERMBARRIERFUNCTIONMATERIAL_H
#define ASYMMETRICCROSSTERMBARRIERFUNCTIONMATERIAL_H

#include "CrossTermBarrierFunctionBase.h"

// Forward Declarations
class AsymmetricCrossTermBarrierFunctionMaterial;

template <>
InputParameters validParams<AsymmetricCrossTermBarrierFunctionMaterial>();

/**
 * AsymmetricCrossTermBarrierFunctionMaterial adds a free energy contribution on the
 * interfaces between arbitrary pairs of phases in an asymmetric way, allowing to tune the
 * magnitude of the free energy density cotribution on both sides of the interface independently.
 */
class AsymmetricCrossTermBarrierFunctionMaterial : public CrossTermBarrierFunctionBase
{
public:
  AsymmetricCrossTermBarrierFunctionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  ///@{ Switching functions and their drivatives
  std::vector<const MaterialProperty<Real> *> _h;
  std::vector<const MaterialProperty<Real> *> _dh;
  std::vector<const MaterialProperty<Real> *> _d2h;
  ///@}
};

#endif // ASYMMETRICCROSSTERMBARRIERFUNCTIONMATERIAL_H
