//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#pragma once

#include "Material.h"
#include "TorchScriptUserObject.h"

/**
 * This material declares properties which are evaluated as
 * based on a torch script neural network.
 */
class TorchScriptMaterial : public Material
{
public:
  static InputParameters validParams();

  TorchScriptMaterial(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Names of the material properties to define
  const std::vector<std::string> _prop_names;

  /// Number of properties that will be defined
  const unsigned int _num_props;

  /// The functions to use for each property
  const std::vector<PostprocessorName> _input_names;

  /// Number of inputs to the neural net
  const unsigned int _num_inputs;

  /// The module input parameters stored as postprocessor values.
  /// TODO: make this more generali n the future
  std::vector<const PostprocessorValue *> _module_inputs;

  /// Vector of all the properties, for now we don't support AD
  std::vector<GenericMaterialProperty<Real, false> *> _properties;

  /// The user object that holds the torch module
  const TorchScriptUserObject & _torch_script_userobject;

  /// Place holder for the inputs to the neural network
  torch::Tensor _input_tensor;

private:
  /**
   * A helper method for evaluating the torch script module and populating the
   * material properties.
   */
  void computeQpValues();
};

#endif
