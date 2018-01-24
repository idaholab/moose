/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef DARCYMATERIAL_H
#define DARCYMATERIAL_H

#include "Material.h"

// Forward Declarations
class DarcyMaterial;

template <>
InputParameters validParams<DarcyMaterial>();

/**
 * Defines the permeability tensor used in Darcy flow
 */
class DarcyMaterial : public Material
{
public:
  DarcyMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// permeability as entered by the user
  RealTensorValue _material_perm;

  /// the Material property that this Material provides
  MaterialProperty<RealTensorValue> & _permeability;
};

#endif // DARCYMATERIAL_H
