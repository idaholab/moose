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

  enum class TransformEnum
  {
    COMPRESS,
    DECOMPRESS,
    LOG10BOUNDED,
    EXP10BOUNDED,
    MINMAX
  };

protected:
  void set_value(bool, bool, bool) override;

private:
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

  /// transform data
  Scalar transform_data(const Scalar & data,
                        const std::vector<Real> & param,
                        TransformEnum transform_type) const;

  /// read in json axes transform name
  std::string json_to_string(std::string key);

  /// read in json axes transform constants
  std::vector<Real> json_to_vector(std::string key);

  ///read 6D grid date from json and store in Torch tensor
  Scalar json_6Dvector_to_torch(std::string key);

  ///read 1D vector of grid points from json and store in Torch tensor
  Scalar json_vector_to_torch(std::string key);

  /// compute interpolated value
  Scalar compute_interpolation(const std::vector<std::pair<Scalar, Scalar>> index_and_fraction,
                               const Scalar grid_values);

  TransformEnum get_transform_enum(const std::string & name) const;

  /// input transform strings
  TransformEnum _stress_transform_enum;
  TransformEnum _temperature_transform_enum;
  TransformEnum _plastic_strain_transform_enum;
  TransformEnum _cell_transform_enum;
  TransformEnum _wall_transform_enum;
  TransformEnum _env_transform_enum;

  /// input transform values
  std::vector<Real> _stress_transform_values;
  std::vector<Real> _temperature_transform_values;
  std::vector<Real> _plastic_strain_transform_values;
  std::vector<Real> _cell_transform_values;
  std::vector<Real> _wall_transform_values;
  std::vector<Real> _env_transform_values;

  /// output transform strings
  std::string _output_transform_name;
  /// output transform values
  std::vector<Real> _output_transform_values;
  /// output transform enum
  TransformEnum _output_transform_enum;

  /// LAROMANCE transforms for input axis
  // @{
  Scalar transform_compress(const Scalar & data, const std::vector<Real> & params) const;
  Scalar transform_decompress(const Scalar & data, const std::vector<Real> & params) const;
  Scalar transform_log10_bounded(const Scalar & data, const std::vector<Real> & params) const;
  Scalar transform_exp10_bounded(const Scalar & data, const std::vector<Real> & params) const;
  Scalar transform_min_max(const Scalar & data, const std::vector<Real> & params) const;
  // @}
};
} // namespace neml2
