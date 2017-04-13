/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CONSTANTANISOTROPICMOBILITY_H
#define CONSTANTANISOTROPICMOBILITY_H

#include "Material.h"

class ConstantAnisotropicMobility;

template <>
InputParameters validParams<ConstantAnisotropicMobility>();

/**
 * ConstantAnisotropicMobility provides a simple RealTensorValue type
 * MaterialProperty that can be used as a mobility in a phase field simulation.
 */
class ConstantAnisotropicMobility : public Material
{
public:
  ConstantAnisotropicMobility(const InputParameters & parameters);

protected:
  virtual void computeProperties(){};
  virtual void initialSetup();

  /// raw tensor values as passed in from the input file
  std::vector<Real> _M_values;

  /// Name of the mobility tensor material property
  MaterialPropertyName _M_name;
  MaterialProperty<RealTensorValue> & _M;
};

#endif // CONSTANTANISOTROPICMOBILITY_H
