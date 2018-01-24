//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TABULATEDFLUIDPROPERTIES_H
#define TABULATEDFLUIDPROPERTIES_H

#include "SinglePhaseFluidPropertiesPT.h"
#include "DelimitedFileReader.h"

class SinglePhaseFluidPropertiesPT;
class BicubicSplineInterpolation;
class TabulatedFluidProperties;

template <>
InputParameters validParams<TabulatedFluidProperties>();

/**
 * Class for fluid properties read from a file.
 *
 * Property values are read from a CSV file containing property data.
 * Monotonically increasing values of pressure and temperature must be included in
 * the data file, specifying the phase space where tabulated fluid properties will
 * be defined. An error is thrown if either temperature or pressure data is not
 * included or not monotonic, and an error is also thrown if this UserObject is
 * requested to provide a fluid property outside this phase space.
 *
 * This class is intended to be used when complicated formulations for density,
 * internal energy or enthalpy are required, which can be computationally expensive.
 * This is particularly the case where the fluid equation of state is based on a
 * Helmholtz free energy that is a function of density and temperature, like that
 * used in CO2FluidProperties. In this case, density must be solved iteratively using
 * pressure and temperature, which increases the computational burden.
 *
 * In these cases, using an interpolation of the tabulated fluid properties can
 * significantly reduce the computational time for computing density, internal energy,
 * and enthalpy.
 *
 * The expected file format for the tabulated fluid properties is now described.
 * The first line must be the header containing the required column
 * names "pressure", "temperature", "density", "enthalpy", "internal_energy" (note: the
 * order is not important, although having pressure and temperature first makes the data
 * easier for a human to read).
 *
 * The data in the pressure and temperature columns must be monotonically increasing. This file
 * format does require duplication of the pressure and temperature data - each pressure value
 * must be included num_T times, while each temperature value is repeated num_p times, where
 * num_T and num_p are the number of temperature and pressure points, respectively. This class
 * will check that the required number of data points have been entered (num_T * num_p).
 *
 * An example of a valid fluid properties file is provided below:
 *
 * pressure, temperature,   density, enthalpy, internal_energy
 *   200000,         275,   3.90056,   -21487,        -72761.7
 *   200000,         277,   3.86573, -19495.4,        -71232.0
 *   200000,         280,   3.83155, -17499.1,        -69697.3
 *   300000,         275,   6.07273, -22728.3,        -73626.5
 *   300000,         277,   6.01721, -20711.5,        -72079.3
 *   300000,         280,   5.96277, -18691.0,        -70527.7
 *
 * and so on.
 *
 * If no tabulated fluid property data file exists, then data for density, internal energy
 * and enthalpy will be generated using the pressure and temperature ranges specified
 * in the input file at the beginning of the simulation.
 *
 * This tabulated data will be written to file in the correct format,
 * enabling suitable data files to be created for future use. There is an upfront
 * computational expense required for this initial data generation, depending on the
 * required number of pressure and temperature points. However, provided that the
 * number of data points required to generate the tabulated data is smaller than the
 * number of times the property members in the FluidProperties UserObject are used,
 * the initial time to generate the data and the subsequent interpolation time can be much
 * less than using the original FluidProperties UserObject.
 *
 * Density, internal_energy and enthalpy and their derivatives wrt pressure and
 * temperature are always calculated using bicubic spline interpolation, while all
 * remaining fluid properties are calculated using the FluidProperties UserObject _fp.
 *
 * A function to write generated data to file using the correct format is provided
 * to allow suitable files of fluid property data to be generated using the FluidProperties
 * module UserObjects.
 */
class TabulatedFluidProperties : public SinglePhaseFluidPropertiesPT
{
public:
  TabulatedFluidProperties(const InputParameters & parameters);
  virtual ~TabulatedFluidProperties();

  virtual void initialSetup() override;

  virtual std::string fluidName() const override;

  virtual Real molarMass() const override;

  virtual Real rho(Real pressure, Real temperature) const override;

  virtual void rho_dpT(
      Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const override;

  virtual Real e(Real pressure, Real temperature) const override;

  virtual void
  e_dpT(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const override;

  virtual void rho_e_dpT(Real pressure,
                         Real temperature,
                         Real & rho,
                         Real & drho_dp,
                         Real & drho_dT,
                         Real & e,
                         Real & de_dp,
                         Real & de_dT) const override;

  virtual Real h(Real p, Real T) const override;

  virtual void
  h_dpT(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const override;

  virtual Real mu(Real pressure, Real temperature) const override;

  virtual void
  mu_dpT(Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const override;

  virtual Real mu_from_rho_T(Real density, Real temperature) const override;

  virtual void mu_drhoT_from_rho_T(Real density,
                                   Real temperature,
                                   Real ddensity_dT,
                                   Real & mu,
                                   Real & dmu_drho,
                                   Real & dmu_dT) const override;

  virtual Real cp(Real pressure, Real temperature) const override;

  virtual Real cv(Real pressure, Real temperature) const override;

  virtual Real c(Real pressure, Real temperature) const override;

  virtual Real k(Real pressure, Real temperature) const override;

  virtual void
  k_dpT(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const override;

  virtual Real k_from_rho_T(Real density, Real temperature) const override;

  virtual Real s(Real pressure, Real temperature) const override;

  virtual Real beta(Real pressure, Real temperature) const override;

  virtual Real henryConstant(Real temperature) const override;

  virtual void henryConstant_dT(Real temperature, Real & Kh, Real & dKh_dT) const override;

protected:
  /**
   * Writes tabulated data to a file.
   * @param file_name name of the file to be written
   */
  void writeTabulatedData(std::string file_name);

  /**
   * Checks that the inputs are within the range of the tabulated data, and throws
   * an error if they are not.
   * @param pressure input pressure (Pa)
   * @param temperature input temperature (K)
   */
  virtual void checkInputVariables(Real & pressure, Real & temperature) const;

  /**
   * Generates a table of fluid properties by looping over pressure and temperature
   * and calculating properties using the FluidProperties UserObject _fp.
   */
  virtual void generateTabulatedData();

  /**
   * Forms a 2D matrix from a single std::vector.
   * @param nrow number of rows in the matrix
   * @param ncol number of columns in the matrix
   * @param vec 1D vector to reshape into a 2D matrix
   * @param[out] 2D matrix formed by reshaping vec
   */
  void reshapeData2D(unsigned int nrow,
                     unsigned int ncol,
                     const std::vector<Real> & vec,
                     std::vector<std::vector<Real>> & mat);

  /// File name of tabulated data file
  FileName _file_name;
  /// Pressure vector
  std::vector<Real> _pressure;
  /// Temperature vector
  std::vector<Real> _temperature;
  /// Tabulated density
  std::vector<std::vector<Real>> _density;
  /// Tabulated internal energy
  std::vector<std::vector<Real>> _internal_energy;
  /// Tabulated enthalpy
  std::vector<std::vector<Real>> _enthalpy;
  /// Interpoled density
  std::unique_ptr<BicubicSplineInterpolation> _density_ipol;
  /// Interpoled internal energy
  std::unique_ptr<BicubicSplineInterpolation> _internal_energy_ipol;
  /// Interpoled enthalpy
  std::unique_ptr<BicubicSplineInterpolation> _enthalpy_ipol;
  /// Derivatives along the boundary
  std::vector<Real> _drho_dp_0, _drho_dp_n, _drho_dT_0, _drho_dT_n;
  std::vector<Real> _de_dp_0, _de_dp_n, _de_dT_0, _de_dT_n;
  std::vector<Real> _dh_dp_0, _dh_dp_n, _dh_dT_0, _dh_dT_n;

  /// Minimum temperature in tabulated data
  Real _temperature_min;
  /// Maximum temperature in tabulated data
  Real _temperature_max;
  /// Minimum pressure in tabulated data
  Real _pressure_min;
  /// Maximum pressure in tabulated data
  Real _pressure_max;
  /// Number of temperature points in the tabulated data
  unsigned int _num_T;
  /// Number of pressure points in the tabulated data
  unsigned int _num_p;
  /// Index for derivatives wrt pressure
  const unsigned int _wrt_p = 1;
  /// Index for derivatives wrt temperature
  const unsigned int _wrt_T = 2;

  /// SinglePhaseFluidPropertiesPT UserObject
  const SinglePhaseFluidPropertiesPT & _fp;

  /// List of required column names to be read
  const std::vector<std::string> _required_columns{
      "pressure", "temperature", "density", "enthalpy", "internal_energy"};
  /// The MOOSE delimited file reader.
  MooseUtils::DelimitedFileReader _csv_reader;
};

#endif /* TABULATEDFLUIDPROPERTIES_H */
