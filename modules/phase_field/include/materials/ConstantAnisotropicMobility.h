//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
