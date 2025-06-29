//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LAROMANCE6DInterpolation.h"

#ifdef NEML2_ENABLED

#include <fstream>
#include <initializer_list>
#include <torch/torch.h>

#include "neml2/tensors/functions/sign.h"
#include "neml2/tensors/functions/abs.h"
#include "neml2/tensors/functions/pow.h"
#include "neml2/tensors/functions/log10.h"
#include "neml2/tensors/functions/clamp.h"
#include "neml2/tensors/functions/stack.h"

namespace neml2
{
register_NEML2_object(LAROMANCE6DInterpolation);

OptionSet
LAROMANCE6DInterpolation::expected_options()
{
  auto options = Model::expected_options();
  options.doc() =
      "Multilinear interpolation over six dimensions (von_mises_stress, temperature, "
      "equivalent_plastic_strain, cell_dislocation_density, wall_dislocation_density, env_factor)";
  // Model inputs
  options.set<VariableName>("equivalent_plastic_strain");
  options.set<VariableName>("von_mises_stress");

  options.set<VariableName>("cell_dislocation_density");
  options.set<VariableName>("wall_dislocation_density");

  options.set<VariableName>("temperature");
  options.set<VariableName>("env_factor");
  // Model Outputs
  options.set_output("output_rate");
  // JSON
  options.set<std::string>("model_file_name");
  options.set<std::string>("model_file_variable_name");
  // jit does not currently work with this
  options.set<bool>("jit") = false;
  options.set("jit").suppressed() = true;
  return options;
}

LAROMANCE6DInterpolation::LAROMANCE6DInterpolation(const OptionSet & options)
  : Model(options),
    _vm_stress(declare_input_variable<Scalar>("von_mises_stress")),
    _temperature(declare_input_variable<Scalar>("temperature")),
    _ep_strain(declare_input_variable<Scalar>("equivalent_plastic_strain")),
    _cell_dd(declare_input_variable<Scalar>("cell_dislocation_density")),
    _wall_dd(declare_input_variable<Scalar>("wall_dislocation_density")),
    _env_fac(declare_input_variable<Scalar>("env_factor")),
    _output_rate(declare_output_variable<Scalar>("output_rate"))
{
  std::string filename = options.get<std::string>("model_file_name");
  std::ifstream model_file(filename.c_str());
  model_file >> _json;

  // storing grid points for indexing.
  // these should be stored differently so that they are all read in at once.  The order of this can
  // get messed up easily
  _stress_grid = json_vector_to_torch("in_stress");
  _temperature_grid = json_vector_to_torch("in_temperature");
  _plastic_strain_grid = json_vector_to_torch("in_plastic_strain");
  _cell_grid = json_vector_to_torch("in_cell");
  _wall_grid = json_vector_to_torch("in_wall");
  _env_grid = json_vector_to_torch("in_env");

  // Read in grid axes transform enums
  _stress_transform_enum = get_transform_enum(json_to_string("in_stress_transform_type"));
  _temperature_transform_enum = get_transform_enum(json_to_string("in_temperature_transform_type"));
  _plastic_strain_transform_enum =
      get_transform_enum(json_to_string("in_plastic_strain_transform_type"));
  _cell_transform_enum = get_transform_enum(json_to_string("in_cell_transform_type"));
  _wall_transform_enum = get_transform_enum(json_to_string("in_wall_transform_type"));
  _env_transform_enum = get_transform_enum(json_to_string("in_env_transform_type"));

  // Read in grid axes transform values
  _stress_transform_values = json_to_vector("in_stress_transform_values");
  _temperature_transform_values = json_to_vector("in_temperature_transform_values");
  _plastic_strain_transform_values = json_to_vector("in_plastic_strain_transform_values");
  _cell_transform_values = json_to_vector("in_cell_transform_values");
  _wall_transform_values = json_to_vector("in_wall_transform_values");
  _env_transform_values = json_to_vector("in_env_transform_values");

  // Storing values for interpolation
  _output_rate_name = options.get<std::string>("model_file_variable_name");
  _grid_values = json_6Dvector_to_torch(_output_rate_name);

  // set up output transforms
  if (_output_rate_name == "out_ep")
  {
    _output_transform_enum = get_transform_enum(json_to_string("out_strain_rate_transform_type"));
    _output_transform_values = json_to_vector("out_strain_rate_transform_values");
  }
  else if (_output_rate_name == "out_cell")
  {
    _output_transform_enum = get_transform_enum(json_to_string("out_cell_rate_transform_type"));
    _output_transform_values = json_to_vector("out_cell_rate_transform_values");
  }
  else if (_output_rate_name == "out_wall")
  {
    _output_transform_enum = get_transform_enum(json_to_string("out_wall_rate_transform_type"));
    _output_transform_values = json_to_vector("out_wall_rate_transform_values");
  }
  else
  {
    throw NEMLException("This ouput variable is not implemented, model_file_variable_name: " +
                        std::string(_output_rate_name));
  }
}

void
LAROMANCE6DInterpolation::request_AD()
{
  // only using first derivatives of out_ep, not out_cell and out_wall
  if (_output_rate_name == "out_ep")
  {
    std::vector<const VariableBase *> inputs = {&_vm_stress};
    _output_rate.request_AD(inputs);
  }
}

void
LAROMANCE6DInterpolation::set_value(bool out, bool /*dout_din*/, bool /*d2out_din2*/)
{
  if (out)
    _output_rate = interpolate_and_transform();
}

LAROMANCE6DInterpolation::TransformEnum
LAROMANCE6DInterpolation::get_transform_enum(const std::string & name) const
{
  if (name == "COMPRESS")
    return TransformEnum::COMPRESS;
  else if (name == "DECOMPRESS")
    return TransformEnum::DECOMPRESS;
  else if (name == "LOG10BOUNDED")
    return TransformEnum::LOG10BOUNDED;
  else if (name == "EXP10BOUNDED")
    return TransformEnum::EXP10BOUNDED;
  else if (name == "MINMAX")
    return TransformEnum::MINMAX;

  throw NEMLException("Unrecognized transform: " + std::string(name));
}

std::pair<Scalar, Scalar>
LAROMANCE6DInterpolation::findLeftIndexAndFraction(const Scalar & grid,
                                                   const Scalar & interp_points) const
{
  // idx is for the left grid point.
  // searchsorted returns the right idx so -1 makes it the left
  auto left_idx = Scalar(torch::searchsorted(grid, interp_points) - 1);

  // this allows us to extrapolate
  left_idx = Scalar(torch::clamp(left_idx, 0, grid.sizes()[0] - 2));

  auto left_coord = grid.batch_index({left_idx});
  auto right_coord =
      grid.batch_index({left_idx + torch::tensor(1, default_integer_tensor_options())});
  auto left_fraction = (right_coord - interp_points) / (right_coord - left_coord);

  return {left_idx, neml2::batch_stack({left_fraction, 1 - left_fraction}, -1)};
}

Scalar
LAROMANCE6DInterpolation::compute_interpolation(
    const std::vector<std::pair<Scalar, Scalar>> index_and_fraction, const Scalar grid_values) const
{
  Scalar result = Scalar::zeros_like(_temperature);
  for (const auto i : {0, 1})
    for (const auto j : {0, 1})
      for (const auto k : {0, 1})
        for (const auto l : {0, 1})
          for (const auto m : {0, 1})
            for (const auto n : {0, 1})
            {
              auto vertex_value =
                  grid_values.index({(index_and_fraction[0].first +
                                      torch::tensor(i, default_integer_tensor_options())),
                                     (index_and_fraction[1].first +
                                      torch::tensor(j, default_integer_tensor_options())),
                                     (index_and_fraction[2].first +
                                      torch::tensor(k, default_integer_tensor_options())),
                                     (index_and_fraction[3].first +
                                      torch::tensor(l, default_integer_tensor_options())),
                                     (index_and_fraction[4].first +
                                      torch::tensor(m, default_integer_tensor_options())),
                                     (index_and_fraction[5].first +
                                      torch::tensor(n, default_integer_tensor_options()))});
              auto weight = index_and_fraction[0].second.select(-1, i) *
                            index_and_fraction[1].second.select(-1, j) *
                            index_and_fraction[2].second.select(-1, k) *
                            index_and_fraction[3].second.select(-1, l) *
                            index_and_fraction[4].second.select(-1, m) *
                            index_and_fraction[5].second.select(-1, n);
              result += vertex_value * weight;
            }
  return result;
}

/// compute interpolated value
Scalar
LAROMANCE6DInterpolation::interpolate_and_transform() const
{
  // These transform constants should be given in the json file.
  const auto cell_dd_transformed =
      transform_data(_cell_dd, _cell_transform_values, _cell_transform_enum);
  const auto wall_dd_transformed =
      transform_data(_wall_dd, _wall_transform_values, _wall_transform_enum);
  const auto vm_stress_transformed =
      transform_data(_vm_stress, _stress_transform_values, _stress_transform_enum);
  const auto ep_strain_transformed =
      transform_data(_ep_strain, _plastic_strain_transform_values, _plastic_strain_transform_enum);
  const auto temperature_transformed =
      transform_data(_temperature, _temperature_transform_values, _temperature_transform_enum);
  const auto env_fac_transformed =
      transform_data(_env_fac, _env_transform_values, _env_transform_enum);

  std::vector<std::pair<Scalar, Scalar>> left_index_weight;
  left_index_weight.push_back(findLeftIndexAndFraction(_stress_grid, vm_stress_transformed));
  left_index_weight.push_back(findLeftIndexAndFraction(_temperature_grid, temperature_transformed));
  left_index_weight.push_back(
      findLeftIndexAndFraction(_plastic_strain_grid, ep_strain_transformed));
  left_index_weight.push_back(findLeftIndexAndFraction(_cell_grid, cell_dd_transformed));
  left_index_weight.push_back(findLeftIndexAndFraction(_wall_grid, wall_dd_transformed));
  left_index_weight.push_back(findLeftIndexAndFraction(_env_grid, env_fac_transformed));
  Scalar interpolated_result = compute_interpolation(left_index_weight, _grid_values);
  Scalar transformed_result =
      transform_data(interpolated_result, _output_transform_values, _output_transform_enum);
  return transformed_result;
}

Scalar
LAROMANCE6DInterpolation::transform_data(const Scalar & data,
                                         const std::vector<double> & param,
                                         TransformEnum transform_type) const
{
  switch (transform_type)
  {
    case TransformEnum::COMPRESS:
      return transform_compress(data, param);

    case TransformEnum::DECOMPRESS:
      return transform_decompress(data, param);

    case TransformEnum::LOG10BOUNDED:
      return transform_log10_bounded(data, param);

    case TransformEnum::EXP10BOUNDED:
      return transform_exp10_bounded(data, param);

    case TransformEnum::MINMAX:
      return transform_min_max(data, param);

    default:
      return data;
  }
}

Scalar
LAROMANCE6DInterpolation::transform_compress(const Scalar & data,
                                             const std::vector<double> & param) const
{
  double factor = param[0];
  double compressor = param[1];
  double original_min = param[2];
  auto d1 = neml2::sign(data) * neml2::pow(neml2::abs(data * factor), compressor);
  auto transformed_data = neml2::log10(1.0 + d1 - original_min);
  return transformed_data;
}

Scalar
LAROMANCE6DInterpolation::transform_decompress(const Scalar & data,
                                               const std::vector<double> & param) const
{
  double factor = param[0];
  double compressor = param[1];
  double original_min = param[2];
  auto d1 = neml2::pow(10.0, data) - 1.0 + original_min;
  auto transformed_data = neml2::sign(d1) * neml2::pow(neml2::abs(d1), 1.0 / compressor) / factor;
  return transformed_data;
}

Scalar
LAROMANCE6DInterpolation::transform_log10_bounded(const Scalar & data,
                                                  const std::vector<double> & param) const

{
  double factor = param[0];
  double lowerbound = param[1];
  double upperbound = param[2];
  double logmin = param[3];
  double logmax = param[4];
  double range = upperbound - lowerbound;
  auto transformed_data =
      range * (neml2::log10(data + factor) - logmin) / (logmax - logmin) + lowerbound;
  return transformed_data;
}

Scalar
LAROMANCE6DInterpolation::transform_exp10_bounded(const Scalar & data,
                                                  const std::vector<double> & param) const
{
  double factor = param[0];
  double lowerbound = param[1];
  double upperbound = param[2];
  double logmin = param[3];
  double logmax = param[4];
  double range = upperbound - lowerbound;
  auto transformed_data =
      (neml2::pow(10.0, ((data - lowerbound) * (logmax - logmin) / range) + logmin) - factor);
  return transformed_data;
}

Scalar
LAROMANCE6DInterpolation::transform_min_max(const Scalar & data,
                                            const std::vector<double> & param) const
{
  double data_min = param[0];
  double data_max = param[1];
  double scaled_min = param[2];
  double scaled_max = param[3];
  auto transformed_data =
      ((data - data_min) / (data_max - data_min)) * (scaled_max - scaled_min) + scaled_min;
  return transformed_data;
}

std::string
LAROMANCE6DInterpolation::json_to_string(const std::string & key) const
{
  if (!_json.contains(key))
    throw NEMLException("The key '" + std::string(key) + "' is missing from the JSON data file.");

  std::string name = _json[key].get<std::string>();
  return name;
}

std::vector<double>
LAROMANCE6DInterpolation::json_to_vector(const std::string & key) const
{
  if (!_json.contains(key))
    throw NEMLException("The key '" + std::string(key) + "' is missing from the JSON data file.");

  std::vector<double> data_vec = _json[key].get<std::vector<double>>();
  return data_vec;
}

Scalar
LAROMANCE6DInterpolation::json_vector_to_torch(const std::string & key) const
{
  if (!_json.contains(key))
    throw NEMLException("The key '" + std::string(key) + "' is missing from the JSON data file.");

  std::vector<double> in_data = _json[key].get<std::vector<double>>();
  return Scalar::create(in_data).clone();
}

Scalar
LAROMANCE6DInterpolation::json_6Dvector_to_torch(const std::string & key) const
{
  using std::vector;
  if (!_json.contains(key))
    throw NEMLException("The key '" + std::string(key) + "' is missing from the JSON data file.");

  vector<vector<vector<vector<vector<vector<double>>>>>> out_data =
      _json[key].get<vector<vector<vector<vector<vector<vector<double>>>>>>>();

  const int64_t sz_l0 = out_data.size();
  const int64_t sz_l1 = out_data[0].size();
  const int64_t sz_l2 = out_data[0][0].size();
  const int64_t sz_l3 = out_data[0][0][0].size();
  const int64_t sz_l4 = out_data[0][0][0][0].size();
  const int64_t sz_l5 = out_data[0][0][0][0][0].size();

  auto check_level_size =
      [](const int64_t current_vec_size, const int64_t sz_level, const std::string & key)
  {
    if (current_vec_size != sz_level)
      throw NEMLException("Incorrect JSON interpolation grid size for '" + key + "'.");
  };

  std::vector<double> linearize_values;
  check_level_size(out_data.size(), sz_l0, key);
  for (auto && level1 : out_data)
  {
    check_level_size(level1.size(), sz_l1, key);
    for (auto && level2 : level1)
    {
      check_level_size(level2.size(), sz_l2, key);
      for (auto && level3 : level2)
      {
        check_level_size(level3.size(), sz_l3, key);
        for (auto && level4 : level3)
        {
          check_level_size(level4.size(), sz_l4, key);
          for (auto && level5 : level4)
          {
            check_level_size(level5.size(), sz_l5, key);
            for (auto && value : level5)
              linearize_values.push_back(value);
          }
        }
      }
    }
  }

  return Scalar::create(linearize_values)
      .batch_reshape({sz_l0, sz_l1, sz_l2, sz_l3, sz_l4, sz_l5})
      .clone();
}

} // namespace neml2

#endif
