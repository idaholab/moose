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
#include "nlohmann/json.h"

namespace neml2
{
class LAROMANCE6DInterpolation : public Model
{
public:
  static OptionSet expected_options();

  LAROMANCE6DInterpolation(const OptionSet & options);

  void request_AD() override;

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
  const Variable<Scalar> & _vm_stress;
  /// Temperature
  const Variable<Scalar> & _temperature;
  /// The creep strain
  const Variable<Scalar> & _ep_strain;
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
                                                     const Scalar & interp_points) const;

  /// compute interpolated value and transform results
  Scalar interpolate_and_transform() const;

  /// transform data
  Scalar transform_data(const Scalar & data,
                        const std::vector<Real> & param,
                        TransformEnum transform_type) const;

  /// read in json axes transform name
  std::string json_to_string(const std::string & key) const;

  /// read in json axes transform constants
  std::vector<Real> json_to_vector(const std::string & key) const;

  ///read 6D grid date from json and store in Torch tensor
  Scalar json_6Dvector_to_torch(const std::string & key) const;

  ///read 1D vector of grid points from json and store in Torch tensor
  Scalar json_vector_to_torch(const std::string & key) const;

  /// compute interpolated value
  Scalar compute_interpolation(const std::vector<std::pair<Scalar, Scalar>> index_and_fraction,
                               const Scalar grid_values) const;

  TransformEnum get_transform_enum(const std::string & name) const;

  /// input transform enums
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

  /// output transform rate name
  std::string _output_rate_name;
  /// output transform values
  std::vector<Real> _output_transform_values;
  /// output transform enum
  TransformEnum _output_transform_enum;

  /// LAROMANCE transforms for input axes and output axis
  // @{
  Scalar transform_compress(const Scalar & data, const std::vector<Real> & params) const;
  Scalar transform_decompress(const Scalar & data, const std::vector<Real> & params) const;
  Scalar transform_log10_bounded(const Scalar & data, const std::vector<Real> & params) const;
  Scalar transform_exp10_bounded(const Scalar & data, const std::vector<Real> & params) const;
  Scalar transform_min_max(const Scalar & data, const std::vector<Real> & params) const;
  // @}
};
} // namespace neml2

#endif
