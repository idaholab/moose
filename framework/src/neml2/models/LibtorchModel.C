//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML2_ENABLED

// libtorch headers
#include <ATen/ops/ones_like.h>

// neml2 headers
#include "neml2/misc/assertions.h"

// moose headers
#include "LibtorchModel.h"

namespace neml2
{
register_NEML2_object(LibtorchModel);

OptionSet
LibtorchModel::expected_options()
{
  auto options = Model::expected_options();
  options.set<std::vector<VariableName>>("inputs");
  options.set<std::vector<VariableName>>("outputs");
  options.set("outputs").doc() = "The scaled neural network output";
  options.set<std::string>("file_path");
  // No jitting :/
  options.set<bool>("jit") = false;
  options.set("jit").suppressed() = true;
  return options;
}

LibtorchModel::LibtorchModel(const OptionSet & options)
  : Model(options),
    _file_path(Moose::DataFileUtils::getPath(options.get<std::string>("file_path"))),
    _surrogate(std::make_unique<torch::jit::script::Module>(torch::jit::load(_file_path.path)))
{
  // inputs
  for (const auto & fv : options.get<std::vector<VariableName>>("inputs"))
    _inputs.push_back(&declare_input_variable<Scalar>(fv));
  for (const auto & fv : options.get<std::vector<VariableName>>("outputs"))
    _outputs.push_back(&declare_output_variable<Scalar>(fv));
}

void
LibtorchModel::to(const torch::TensorOptions & options)
{
  Model::to(options);

  if (options.has_device())
    _surrogate->to(options.device());

  if (options.has_dtype())
    _surrogate->to(torch::Dtype(caffe2::typeMetaToScalarType(options.dtype())));
}

void
LibtorchModel::request_AD()
{
  std::vector<const VariableBase *> inputs;
  for (size_t i = 0; i < _inputs.size(); ++i)
    inputs.push_back(_inputs[i]);

  for (size_t i = 0; i < _outputs.size(); ++i)
    _outputs[i]->request_AD(inputs);
}

void
LibtorchModel::set_value(bool out, bool /*dout_din*/, bool /*d2out_din2*/)
{
  if (out)
  {
    std::vector<at::Tensor> values;
    auto first_batch_dim = _inputs[0]->batch_dim();
    for (size_t i = 0; i < _inputs.size(); ++i)
    {
      // assert that all inputs have the same batch dimension
      neml_assert(_inputs[i]->batch_dim() == first_batch_dim);
      values.push_back(_inputs[i]->value());
    }

    auto x = Tensor(torch::transpose(torch::vstack(at::ArrayRef<at::Tensor>(
                                         values.data(), static_cast<int64_t>(values.size()))),
                                     0,
                                     1),
                    _inputs[0]->batch_dim());

    // Feed forward the neural network and process the output
    auto temp = _surrogate->forward({x}).toTensor().squeeze();
    auto y0 =
        (temp.dim() == 1) ? temp.view({temp.size(0), 1}).transpose(0, 1) : temp.transpose(0, 1);

    for (size_t i = 0; i < _outputs.size(); ++i)
      *_outputs[i] = Scalar(y0[i], _inputs[0]->batch_dim());
  }
}

}

#endif
