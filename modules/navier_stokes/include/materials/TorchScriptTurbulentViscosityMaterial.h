//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#pragma once

#include "Material.h"
#include "TorchScriptUserObject.h"

/**
 * This material declares properties which are evaluated as
 * based on a torch script neural network.
 */
class TorchScriptTurbulentViscosityMaterial : public Material
{
public:
  static InputParameters validParams();

  TorchScriptTurbulentViscosityMaterial(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Dimmension of the mesh
  const unsigned int _mesh_dimension;

  /// x-velocity
  const Moose::Functor<ADReal> & _u_var;
  /// y-velocity
  const Moose::Functor<ADReal> * _v_var;
  /// z-velocity
  const Moose::Functor<ADReal> * _w_var;

  /// The user object that holds the torch module
  const TorchScriptUserObject & _torch_script_userobject;

  /// Place holder for the inputs to the neural network
  torch::Tensor _input_tensor;

  /// Vector of all the properties, for now we don't support AD
  GenericMaterialProperty<Real, false> * _properties;

private:
  /**
   * A helper method for evaluating the torch script module and populating the
   * material properties.
   */
  void computeQpValues();
};

#endif
