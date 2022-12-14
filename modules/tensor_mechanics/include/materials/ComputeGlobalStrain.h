//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "RankTwoTensorForward.h"

// Forward Declarations
class GlobalStrainUserObjectInterface;

/**
 * ComputeGlobalStrain calculates the global strain tensor from the scalar variables
 */
class ComputeGlobalStrain : public Material
{
public:
  static InputParameters validParams();

  ComputeGlobalStrain(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;

  virtual void computeQpProperties() override;

  /// Base name prepended to material property name
  const std::string _base_name;
  const VariableValue & _scalar_global_strain;
  MaterialProperty<RankTwoTensor> & _global_strain;

  const GlobalStrainUserObjectInterface & _pst;
  const VectorValue<bool> & _periodic_dir;

  const unsigned int _dim;

  /// Number of displacement variables
  const unsigned int _ndisp;
};
