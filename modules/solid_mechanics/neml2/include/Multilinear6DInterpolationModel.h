// Copyright 2024, UChicago Argonne, LLC
// All Rights Reserved
// Software Name: NEML2 -- the New Engineering material Model Library, version 2
// By: Argonne National Laboratory
// OPEN SOURCE LICENSE (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

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

  ///read 6D grid date from json and store in Torch tensor
  Scalar json_6Dvector_to_torch(std::string key);

private:
  ///read 1D vector of grid points from json and store in Torch tensor
  Scalar json_vector_to_torch(std::string key);
  /// compute interpolated value
  Scalar compute_interpolation(const std::vector<std::pair<Scalar, Scalar>> index_and_fraction,
                               const Scalar grid_values);
};
} // namespace neml2
