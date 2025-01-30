#pragma once

#include "neml2/models/Model.h"
#include "nlohmann/json.h"

namespace neml2
{
class Multilinear6DInterpolationModel : public Model
{
public:
  Multilinear6DInterpolationModel(const OptionSet & options);

  static OptionSet expected_options();

protected:
  void set_value(bool, bool, bool) override;

  /// grid for interpolation
  Scalar _stress_grid;
  Scalar _temperature_grid;
  Scalar _plastic_strain_grid;
  Scalar _cell_grid;
  Scalar _wall_grid;
  Scalar _env_grid;

  /// grid values being interpolated
  Scalar _grid_values;

  /// Model input for interpolation
  // @{
  /// The von Mises stress
  const Variable<Scalar> & _s;
  /// Temperature
  const Variable<Scalar> & _T;
  /// The creep strain
  const Variable<Scalar> & _ep;
  /// cell dislocation density
  const Variable<Scalar> & _cell_dd;
  /// wall dislocation density
  const Variable<Scalar> & _wall_dd;
  /// environmental factor
  const Variable<Scalar> & _env_fac;
  // @}

  /// Model output
  // @{
  /// output rate
  Variable<Scalar> & _output_rate;
  // @}

  /// JSON object containing interpolation grid and values
  nlohmann::json _json;

  /// find index of input point
  std::pair<Scalar, Scalar> findLeftIndexAndFraction(const Scalar & grid,
                                                     const Scalar & interp_points);

  /// compute interpolated value and transform results
  Scalar interpolate_and_transform();

  /// transform output data
  virtual Scalar transform(const Scalar & data) = 0;

  /// read in json axes transform name
  std::string json_to_string(std::string key);

  /// read in json axes transform constants
  std::vector<Real> json_to_vector(std::string key);

  ///read 6D grid date from json and store in Torch tensor
  Scalar json_6Dvector_to_torch(std::string key);

private:
  ///read 1D vector of grid points from json and store in Torch tensor
  Scalar json_vector_to_torch(std::string key);

  /// compute interpolated value
  Scalar compute_interpolation(const std::vector<std::pair<Scalar, Scalar>> index_and_fraction,
                               const Scalar grid_values);

  std::string _stress_transform_name;
  std::string _temperature_transform_name;
  std::string _plastic_strain_transform_name;
  std::string _cell_transform_name;
  std::string _wall_transform_name;
  std::string _env_transform_name;

  std::vector<Real> _stress_transform_values;
  std::vector<Real> _temperature_transform_values;
  std::vector<Real> _plastic_strain_transform_values;
  std::vector<Real> _cell_transform_values;
  std::vector<Real> _wall_transform_values;
  std::vector<Real> _env_transform_values;

  /// LAROMANCE transforms for input axis
  // @{
  Scalar transform_compress(const Scalar & data, const std::vector<Real> & params) const;
  Scalar transform_log10_bounded(const Scalar & data, const std::vector<Real> & params) const;
  Scalar transform_min_max(const Scalar & data, const std::vector<Real> & params) const;
  // @}
};
} // namespace neml2
