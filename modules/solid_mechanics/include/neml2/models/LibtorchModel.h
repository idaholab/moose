//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef NEML2_ENABLED

#include "neml2/models/Model.h"
#include <torch/script.h>
#include "DataFileUtils.h"
namespace neml2
{

/**
 * Evaluate a pretrained libtorch model in `.pt` format, such as a neural network.
 * Evaluates models with an arbitrary number of inputs and maps them to an arbitrary number of
 * outputs.
 */
class LibtorchModel : public Model
{
public:
  static OptionSet expected_options();

  LibtorchModel(const OptionSet & options);

  /**
   * @brief Override the base implementation to additionally send the model loaded from torch script
   * to different device and dtype.
   */
  virtual void to(const torch::TensorOptions & options) override;

  virtual void request_AD() override;

protected:
  virtual void set_value(bool out, bool dout_din, bool d2out_din2) override;

  // Input variable vector
  std::vector<const Variable<Scalar> *> _inputs;
  // Output vector
  std::vector<Variable<Scalar> *> _outputs;
  Moose::DataFileUtils::Path _file_path;
  /// We need to use a pointer here because forward is not const qualified
  std::unique_ptr<torch::jit::script::Module> _surrogate;
};

}

#endif
