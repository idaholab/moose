//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEGLOBALSTRAIN_H
#define COMPUTEGLOBALSTRAIN_H

#include "Material.h"

class ComputeGlobalStrain;
class RankTwoTensor;

template <>
InputParameters validParams<ComputeGlobalStrain>();

/**
 * ComputeGlobalStrain is the base class for strain tensors using incremental formulations
 */
class ComputeGlobalStrain : public Material
{
public:
  ComputeGlobalStrain(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeProperties();

  ///Base name prepended to material property name
  std::string _base_name;
  VariableValue & _scalar_global_strain;
  Order _scalar_global_strain_order;
  MaterialProperty<RankTwoTensor> & _global_strain;
};

#endif // COMPUTEGLOBALSTRAIN_H
