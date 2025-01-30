#include "Multilinear6DInterpolationModel.h"
#include <fstream>
#include <initializer_list>
#include "neml2/misc/math.h"

namespace neml2
{

OptionSet
Multilinear6DInterpolationModel::expected_options()
{
  auto options = Model::expected_options();
  options.doc() = "Multilinear interpolation over six dimensions (von_mises_stress, temperature, "
                  "equivalent_plastic_strain, cell_dd_density, wall_dd_density, env_factor)";
  // Model inputs
  options.set<VariableName>("equivalent_plastic_strain") = VariableName("state", "ep");
  options.set<VariableName>("von_mises_stress") = VariableName("state", "s");

  options.set<VariableName>("cell_dd_density") = VariableName("old_state", "cell_dd");
  options.set<VariableName>("wall_dd_density") = VariableName("old_state", "wall_dd");

  options.set<VariableName>("temperature") = VariableName("forces", "T");
  options.set<VariableName>("env_factor") = VariableName("forces", "env_fac");
  // Model Outputs
  options.set_output("output_rate");
  // JSON
  options.set<std::string>("model_file_name");
  options.set<std::string>("model_file_variable_name");

  options.set<bool>("_use_AD_first_derivative") = true;
  options.set<bool>("_use_AD_second_derivative") = true;
  return options;
}

Multilinear6DInterpolationModel::Multilinear6DInterpolationModel(const OptionSet & options)
  : Model(options),
    _s(declare_input_variable<Scalar>("von_mises_stress")),
    _T(declare_input_variable<Scalar>("temperature")),
    _ep(declare_input_variable<Scalar>("equivalent_plastic_strain")),
    _cell_dd(declare_input_variable<Scalar>("cell_dd_density")),
    _wall_dd(declare_input_variable<Scalar>("wall_dd_density")),
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
  // Read in grid axes transform strings
  _stress_transform_name = json_to_string("in_stress_transform_type");
  _temperature_transform_name = json_to_string("in_temperature_transform_type");
  _plastic_strain_transform_name = json_to_string("in_plastic_strain_transform_type");
  _cell_transform_name = json_to_string("in_cell_transform_type");
  _wall_transform_name = json_to_string("in_wall_transform_type");
  _env_transform_name = json_to_string("in_env_transform_type");
  // Read in grid axes transform values
  _stress_transform_values = json_to_vector("in_stress_transform_values");
  _temperature_transform_values = json_to_vector("in_temperature_transform_values");
  _plastic_strain_transform_values = json_to_vector("in_plastic_strain_transform_values");
  _cell_transform_values = json_to_vector("in_cell_transform_values");
  _wall_transform_values = json_to_vector("in_wall_transform_values");
  _env_transform_values = json_to_vector("in_env_transform_values");
  // Storing values for interpolation
  std::string filename_variable = options.get<std::string>("model_file_variable_name");
  _grid_values = json_6Dvector_to_torch(filename_variable);
}

void
Multilinear6DInterpolationModel::set_value(bool /*out*/, bool /*dout_din*/, bool /*d2out_din2*/)
{
  neml_assert_dbg("Multilinear6DInterpolationModel should never be called, it is only base class "
                  "with helper functions.");
}

std::pair<Scalar, Scalar>
Multilinear6DInterpolationModel::findLeftIndexAndFraction(const Scalar & grid,
                                                          const Scalar & interp_points)
{
  // idx is for the left grid point.
  // searchsorted returns the right idx so -1 makes it the left
  auto left_idx = torch::searchsorted(grid, interp_points) - 1;

  // this allows us to extrapolate
  left_idx = torch::clamp(left_idx, 0, grid.sizes()[0] - 2);

  Scalar left_coord = grid.index({left_idx});
  Scalar right_coord = grid.index({left_idx + torch::tensor(1, default_integer_tensor_options())});
  Scalar left_fraction = (right_coord - interp_points) / (right_coord - left_coord);

  // derivative (only needed for stress):
  // Scalar left_fraction_deriv = right_coord / (right_coord - left_coord);
  // Scalar right_fraction_deriv = -left_coord / (right_coord - left_coord);

  neml_assert_dbg((!torch::all(left_fraction.gt(1)).item<bool>() &&
                   !torch::all(left_fraction.lt(0)).item<bool>()),
                  "Interpolated value outside interpolation grid.  This exception is only thrown "
                  "in debug.  Running in opt will perform extrapolation.");

  return std::pair<Scalar, Scalar>(left_idx, torch::stack({left_fraction, 1 - left_fraction}, -1));
}

Scalar
Multilinear6DInterpolationModel::compute_interpolation(
    const std::vector<std::pair<Scalar, Scalar>> index_and_fraction, const Scalar grid_values)
{
  Scalar result = Scalar::zeros_like(_T);
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
Multilinear6DInterpolationModel::interpolate_and_transform()
{
  // These transform constants should be given in the json file.
  auto cell_dd_transformed = transform_compress(_cell_dd, _cell_transform_values);
  auto wall_dd_transformed = transform_min_max(_wall_dd, _wall_transform_values);
  auto s_transformed = transform_min_max(_s, _stress_transform_values);
  auto ep_transformed = transform_log10_bounded(_ep, _plastic_strain_transform_values);
  auto T_transformed = transform_min_max(_T, _temperature_transform_values);
  auto env_fac_transformed = transform_min_max(_env_fac, _env_transform_values);

  std::vector<std::pair<Scalar, Scalar>> left_index_weight;
  left_index_weight.push_back(findLeftIndexAndFraction(_stress_grid, s_transformed));
  left_index_weight.push_back(findLeftIndexAndFraction(_temperature_grid, T_transformed));
  left_index_weight.push_back(findLeftIndexAndFraction(_plastic_strain_grid, ep_transformed));
  left_index_weight.push_back(findLeftIndexAndFraction(_cell_grid, cell_dd_transformed));
  left_index_weight.push_back(findLeftIndexAndFraction(_wall_grid, wall_dd_transformed));
  left_index_weight.push_back(findLeftIndexAndFraction(_env_grid, env_fac_transformed));
  Scalar interpolated_result = compute_interpolation(left_index_weight, _grid_values);
  Scalar transformed_result = transform(interpolated_result);
  return transformed_result;
}

std::string
Multilinear6DInterpolationModel::json_to_string(std::string key)
{
  neml_assert_dbg(_json.contains(key), "The key '", key, "' is missing from the JSON data file.");
  std::string name = _json[key].template get<std::string>();
  return name;
}

std::vector<Real>
Multilinear6DInterpolationModel::json_to_vector(std::string key)
{
  neml_assert_dbg(_json.contains(key), "The key '", key, "' is missing from the JSON data file.");
  std::vector<Real> data_vec = _json[key].template get<std::vector<Real>>();
  return data_vec;
}

Scalar
Multilinear6DInterpolationModel::json_vector_to_torch(std::string key)
{
  neml_assert_dbg(_json.contains(key), "The key '", key, "' is missing from the JSON data file.");
  std::vector<Real> in_data = _json[key].template get<std::vector<Real>>();
  const long long data_dim = in_data.size();
  return Scalar(torch::from_blob(in_data.data(), {data_dim}).clone());
}

Scalar
Multilinear6DInterpolationModel::json_6Dvector_to_torch(std::string key)
{
  neml_assert_dbg(_json.contains(key), "The key '", key, "' is missing from the JSON data file.");

  std::vector<std::vector<std::vector<std::vector<std::vector<std::vector<Real>>>>>> out_data =
      _json[key]
          .template get<
              std::vector<std::vector<std::vector<std::vector<std::vector<std::vector<Real>>>>>>>();

  std::vector<Real> linearize_values;
  for (auto && level0 : out_data)
    for (auto && level1 : level0)
      for (auto && level2 : level1)
        for (auto && level3 : level2)
          for (auto && level4 : level3)
            for (auto && value : level4)
              linearize_values.push_back(value);

  long long sz_l0 = out_data.size();
  long long sz_l1 = out_data[0].size();
  long long sz_l2 = out_data[0][0].size();
  long long sz_l3 = out_data[0][0][0].size();
  long long sz_l4 = out_data[0][0][0][0].size();
  long long sz_l5 = out_data[0][0][0][0][0].size();
  return Scalar(
      torch::from_blob(linearize_values.data(), {sz_l0, sz_l1, sz_l2, sz_l3, sz_l4, sz_l5})
          .clone());
}

Scalar
Multilinear6DInterpolationModel::transform_compress(const Scalar & data,
                                                    const std::vector<Real> & param) const
{
  Real factor = param[0];
  Real compressor = param[1];
  Real original_min = param[2];
  auto d1 = math::sign(data) * math::pow(math::abs(data * factor), compressor);
  auto transformed_data = math::log10(1.0 + d1 - original_min);
  return transformed_data;
}

Scalar
Multilinear6DInterpolationModel::transform_log10_bounded(const Scalar & data,
                                                         const std::vector<Real> & param) const

{
  Real factor = param[0];
  Real lowerbound = param[1];
  Real upperbound = param[2];
  Real logmin = param[3];
  Real logmax = param[4];

  Real range = upperbound - lowerbound;
  auto transformed_data =
      range * (math::log10(data + factor) - logmin) / (logmax - logmin) + lowerbound;
  return transformed_data;
}

Scalar
Multilinear6DInterpolationModel::transform_min_max(const Scalar & data,
                                                   const std::vector<Real> & param) const
{
  Real data_min = param[0];
  Real data_max = param[1];
  Real scaled_min = param[2];
  Real scaled_max = param[3];
  auto transformed_data =
      ((data - data_min) / (data_max - data_min)) * (scaled_max - scaled_min) + scaled_min;
  return transformed_data;
}

} // namespace neml2
