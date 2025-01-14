//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SinglePhaseFluidProperties.h"
#include "DelimitedFileReader.h"

#include "libmesh/utility.h"

class SinglePhaseFluidProperties;
class BidimensionalInterpolation;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * Class for fluid properties read from a tabulation in a file.
 */
class TabulatedFluidProperties : public SinglePhaseFluidProperties
{
public:
  static InputParameters validParams();

  TabulatedFluidProperties(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void constructInterpolation() = 0;

  virtual std::string fluidName() const override;

  virtual Real molarMass() const override;

  virtual Real rho_from_p_T(Real pressure, Real temperature) const override;

  virtual void rho_from_p_T(
      Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const override;
  virtual void rho_from_p_T(const ADReal & pressure,
                            const ADReal & temperature,
                            ADReal & rho,
                            ADReal & drho_dp,
                            ADReal & drho_dT) const override;

  virtual Real rho_from_p_s(Real p, Real s) const override;
  virtual void
  rho_from_p_s(Real p, Real s, Real & rho, Real & drho_dp, Real & drho_ds) const override;

  virtual Real v_from_p_T(Real pressure, Real temperature) const override;

  virtual void
  v_from_p_T(Real pressure, Real temperature, Real & v, Real & dv_dp, Real & dv_dT) const override;

  virtual Real e_from_p_T(Real pressure, Real temperature) const override;

  virtual void
  e_from_p_T(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const override;

  virtual Real e_from_p_rho(Real pressure, Real rho) const override;

  virtual void
  e_from_p_rho(Real pressure, Real rho, Real & e, Real & de_dp, Real & de_drho) const override;

  virtual Real T_from_p_rho(Real pressure, Real rho) const;

  virtual void T_from_p_rho(Real pressure, Real rho, Real & T, Real & dT_dp, Real & dT_drho) const;

  virtual Real h_from_p_T(Real p, Real T) const override;

  virtual ADReal h_from_p_T(const ADReal & pressure, const ADReal & temperature) const override;

  virtual void
  h_from_p_T(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const override;

  virtual Real mu_from_p_T(Real pressure, Real temperature) const override;

  virtual void mu_from_p_T(
      Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const override;

  virtual Real cp_from_p_T(Real pressure, Real temperature) const override;

  virtual void cp_from_p_T(
      Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const override;

  using SinglePhaseFluidProperties::cp_from_p_T;

  virtual Real cv_from_p_T(Real pressure, Real temperature) const override;
  virtual void cv_from_p_T(
      Real pressure, Real temperature, Real & cv, Real & dcv_dp, Real & dcv_dT) const override;

  virtual Real c_from_p_T(Real pressure, Real temperature) const override;
  virtual void
  c_from_p_T(Real pressure, Real temperature, Real & c, Real & dc_dp, Real & dc_dT) const override;

  virtual Real k_from_p_T(Real pressure, Real temperature) const override;

  virtual void
  k_from_p_T(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const override;

  virtual Real s_from_p_T(Real pressure, Real temperature) const override;

  virtual void s_from_p_T(Real p, Real T, Real & s, Real & ds_dp, Real & ds_dT) const override;

  /**
   * The following routines are simply forwarded to the 'fp' companion FluidProperties
   * as they are not included in the tabulations presently
   */
  virtual std::vector<Real> henryCoefficients() const override;

  virtual Real vaporPressure(Real temperature) const override;

  virtual void vaporPressure(Real temperature, Real & psat, Real & dpsat_dT) const override;

  virtual Real vaporTemperature(Real pressure) const override;
  virtual void vaporTemperature(Real pressure, Real & Tsat, Real & dTsat_dp) const override;

  virtual Real T_from_p_h(Real pressure, Real enthalpy) const override;
  virtual ADReal T_from_p_h(const ADReal & pressure, const ADReal & enthalpy) const override;

  virtual Real triplePointPressure() const override;
  virtual Real triplePointTemperature() const override;
  virtual Real criticalPressure() const override;
  virtual Real criticalTemperature() const override;
  virtual Real criticalDensity() const override;

  /**
   * Derivatives like dc_dv & dc_de are computed using the chain rule
   * dy/dx(p,T) = dy/dp * dp/dx + dy/dT * dT/dx
   * where y = c, cp, cv... & x = v, e
   */
  virtual Real p_from_v_e(Real v, Real e) const override;
  virtual void p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const override;
  virtual Real T_from_v_e(Real v, Real e) const override;
  virtual void T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const override;
  virtual Real c_from_v_e(Real v, Real e) const override;
  virtual void c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const override;
  virtual Real cp_from_v_e(Real v, Real e) const override;
  virtual void cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const override;
  virtual Real cv_from_v_e(Real v, Real e) const override;
  virtual void cv_from_v_e(Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const override;
  virtual Real mu_from_v_e(Real v, Real e) const override;
  virtual void mu_from_v_e(Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const override;
  virtual Real k_from_v_e(Real v, Real e) const override;
  virtual void k_from_v_e(Real v, Real e, Real & k, Real & dk_dv, Real & dk_de) const override;
  virtual Real g_from_v_e(Real v, Real e) const override;
  virtual Real e_from_v_h(Real v, Real h) const override;
  virtual void e_from_v_h(Real v, Real h, Real & e, Real & de_dv, Real & de_dh) const override;
  virtual Real T_from_h_s(Real h, Real s) const;
  virtual Real T_from_p_s(Real p, Real s) const;
  virtual void T_from_p_s(Real p, Real s, Real & T, Real & dT_dp, Real & dT_ds) const;
  virtual Real s_from_v_e(Real v, Real e) const override;
  virtual Real s_from_h_p(Real h, Real pressure) const override;
  virtual void
  s_from_h_p(Real h, Real pressure, Real & s, Real & ds_dh, Real & ds_dp) const override;

  /// AD implementations needed
  using SinglePhaseFluidProperties::c_from_v_e;
  using SinglePhaseFluidProperties::p_from_v_e;
  using SinglePhaseFluidProperties::T_from_v_e;

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
  template <typename T>
  void checkInputVariables(T & pressure, T & temperature) const;

  /**
   * Checks that the inputs are within the range of the tabulated data, and throws
   * an error if they are not.
   * @param v specific volume (m3/kg)
   * @param e specific internal energy (J/kg)
   */
  template <typename T>
  void checkInputVariablesVE(T & v, T & e) const;

  /**
   * Checks initial guess for Newton Method
   */
  virtual void checkInitialGuess() const;

  /// Read tabulation data from file
  void readFileTabulationData(bool use_pT);

  /// Check that the tabulation grids in the file are correct (no repeats etc)
  /// @param v1 the first grid axis (pressure for pT grid)
  /// @param v2 the second grid axis (temperature for pT grid)
  /// @param file_name the name of the tabulation file
  void checkFileTabulationGrids(std::vector<Real> & v1,
                                std::vector<Real> & v2,
                                const std::string & file_name,
                                const std::string & v1_name,
                                const std::string & v2_name);

  /**
   * Generates a table of fluid properties by looping over pressure and temperature
   * and calculating properties using the FluidProperties UserObject _fp.
   */
  virtual void generateTabulatedData();

  /**
   * Generates a table of fluid properties by looping over specific volume and internal energy
   * and calculating properties using the FluidProperties UserObject _fp.
   */
  virtual void generateVETabulatedData();

  /// Retrieves the index for each property in the vector of interpolations
  void computePropertyIndicesInInterpolationVectors();

  /// Create (or reset) the grid vectors for the specific volume and internal energy interpolations
  /// The order of priority for determining the range boundaries in v and e:
  /// - if user-specified, use _v_min/max and _e_min/max
  /// - if reading a (v,e) interpolation, the bounds of that range
  /// - if a _fp exist find the min/max v/e from T_min/max and p_min/max
  void createVGridVector();
  void createVEGridVectors();
  /// Create (or reset) the grid vectors for the specific volume and enthalpy interpolation
  /// The order of priority for determining the range boundaries in v and h:
  /// - if user-specified, use _v_min/max and _e_min/max
  /// - if a _fp exist find the min/max v/e from T_min/max and p_min/max
  /// - if reading a (p,T) tabulation, the bounds of the enthalpy grid
  /// - if reading a (v,e) tabulation, the bounds of the enthalpy grid
  void createVHGridVectors();

  /// Standardized error message for missing interpolation
  void missingVEInterpolationError(const std::string & function_name) const;

  // Utility to forward errors related to fluid properties methods not implemented
  [[noreturn]] void FluidPropertiesForwardError(const std::string & desired_routine) const;

  /// File name of input tabulated data file
  FileName _file_name_in;
  /// File name of input (v,e) tabulated data file
  FileName _file_name_ve_in;
  /// File name of output tabulated data file
  FileName _file_name_out;
  /// File name of output (v,e) tabulated data file
  FileName _file_name_ve_out;
  /// Whether to save a generated fluid properties file to disk
  const bool _save_file;

  /// Pressure vector
  std::vector<Real> _pressure;
  /// Temperature vector
  std::vector<Real> _temperature;
  /// Specific volume vector
  std::vector<Real> _specific_volume;
  /// Specific internal energy vector
  std::vector<Real> _internal_energy;
  /// Specific enthalpy vector
  std::vector<Real> _enthalpy;

  /// Whether to create direct (p,T) interpolations
  const bool _create_direct_pT_interpolations;
  /// Whether to create direct (v,e) interpolations
  const bool _create_direct_ve_interpolations;

  /// Tabulated fluid properties (read from file OR computed from _fp)
  std::vector<std::vector<Real>> _properties;
  /// Tabulated fluid properties in (v,e) (read from file OR computed from _fp)
  std::vector<std::vector<Real>> _properties_ve;

  /// Vector of bi-dimensional interpolation of fluid properties
  std::vector<std::unique_ptr<BidimensionalInterpolation>> _property_ipol;
  /// Vector of bi-dimensional interpolation of fluid properties directly in (v,e)
  std::vector<std::unique_ptr<BidimensionalInterpolation>> _property_ve_ipol;

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

  /// SinglePhaseFluidPropertiesPT UserObject
  const SinglePhaseFluidProperties * const _fp;
  /// Whether to allow a fp object when a tabulation is in use
  const bool _allow_fp_and_tabulation;

  /// Properties to be interpolated entered in the input file
  MultiMooseEnum _interpolated_properties_enum;
  /// List of properties to be interpolated
  std::vector<std::string> _interpolated_properties;
  /// Set of flags to note whether a property is to be interpolated
  bool _interpolate_density;
  bool _interpolate_enthalpy;
  bool _interpolate_internal_energy;
  bool _interpolate_viscosity;
  bool _interpolate_k;
  bool _interpolate_c;
  bool _interpolate_cp;
  bool _interpolate_cv;
  bool _interpolate_entropy;
  bool _interpolate_pressure;
  bool _interpolate_temperature;

  /// Index of each property
  unsigned int _density_idx;
  unsigned int _enthalpy_idx;
  unsigned int _internal_energy_idx;
  unsigned int _viscosity_idx;
  unsigned int _k_idx;
  unsigned int _c_idx;
  unsigned int _cp_idx;
  unsigned int _cv_idx;
  unsigned int _entropy_idx;
  unsigned int _p_idx;
  unsigned int _T_idx;

  /// The MOOSE delimited file reader.
  MooseUtils::DelimitedFileReader _csv_reader;

  /// if the lookup table p(v, e) and T(v, e) should be constructed
  bool _construct_pT_from_ve;
  /// if the lookup table p(v, h) and T(v, h) should be constructed
  bool _construct_pT_from_vh;
  /// keeps track of whether initialSetup has been performed
  bool _initial_setup_done;
  /// Number of specific volume points in the tabulated data
  unsigned int _num_v;
  /// Number of internal energy points in tabulated data
  unsigned int _num_e;
  /// log-space the specific volume interpolation grid axis instead of linear
  bool _log_space_v;
  /// log-space the internal energy interpolation grid axis instead of linear
  bool _log_space_e;
  /// log-space the enthalpy interpolation grid axis instead of linear
  bool _log_space_h;

  /// User-selected out-of-bounds behavior
  MooseEnum _OOBBehavior;
  /// Enum specifying all the behavior on out of bounds data options
  enum OOBBehavior
  {
    Ignore,
    Throw,
    DeclareInvalid,
    WarnInvalid,
    SetToClosestBound
  };

  /// Bi-dimensional interpolation of temperature from (v,e)
  std::unique_ptr<BidimensionalInterpolation> _T_from_v_e_ipol;

  /// Bi-dimensional interpolation of pressure from (v,e)
  std::unique_ptr<BidimensionalInterpolation> _p_from_v_e_ipol;

  /// Bi-dimensional interpolation of temperature from (v,h)
  std::unique_ptr<BidimensionalInterpolation> _T_from_v_h_ipol;

  /// Bidimensional interpolation of pressure from (v,h)
  std::unique_ptr<BidimensionalInterpolation> _p_from_v_h_ipol;

  /// Whether the specific internal energy bounds were set by the user
  bool _e_bounds_specified;
  /// Whether the specific volume bounds were set by the user
  bool _v_bounds_specified;
  /// Minimum internal energy in tabulated data (can be user-specified)
  Real _e_min;
  /// Maximum internal energy in tabulated data (can be user-specified)
  Real _e_max;
  /// Minimum specific volume in tabulated data (can be user-specified)
  Real _v_min;
  /// Maximum specific volume in tabulated data (can be user-specified)
  Real _v_max;
  /// Minimum specific enthalpy in tabulated data
  Real _h_min;
  /// Maximum specific enthalpy in tabulated data
  Real _h_max;
};

#pragma GCC diagnostic pop
