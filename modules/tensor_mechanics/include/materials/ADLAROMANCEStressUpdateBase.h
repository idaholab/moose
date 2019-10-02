//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADRadialReturnCreepStressUpdateBase.h"

#define usingADLAROMANCEStressUpdateBase                                                           \
  usingRadialReturnCreepStressUpdateBaseMembers;                                                   \
  using ADLAROMANCEStressUpdateBase<compute_stage>::getTransform;                                  \
  using ADLAROMANCEStressUpdateBase<compute_stage>::getTransformCoefs;                             \
  using ADLAROMANCEStressUpdateBase<compute_stage>::getInputLimits;                                \
  using ADLAROMANCEStressUpdateBase<compute_stage>::getCoefs

template <ComputeStage>
class ADLAROMANCEStressUpdateBase;

declareADValidParams(ADLAROMANCEStressUpdateBase);

enum class ROMInputTransform
{
  LINEAR,
  LOG,
  EXP
};

template <ComputeStage compute_stage>
class ADLAROMANCEStressUpdateBase : public ADRadialReturnCreepStressUpdateBase<compute_stage>
{
public:
  ADLAROMANCEStressUpdateBase(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;

  virtual void initQpStatefulProperties() override;

  virtual ADReal computeResidual(const ADReal & effective_trial_stress,
                                 const ADReal & scalar) override;

  virtual ADReal computeDerivative(const ADReal & /*effective_trial_stress*/,
                                   const ADReal & /*scalar*/) override
  {
    return _derivative;
  }

  virtual void computeStressFinalize(const ADRankTwoTensor & plastic_strain_increment) override;

  /**
   * Compute the limiting value of the time step for this material
   *
   * @return Limiting time step
   */
  virtual Real computeTimeStepLimit() override;

  /**
   *Computes the ROM Strain rate
   * @param dt Timestep size
   * @param mobile_dislocations_old Mobile dislocation density from previous timestep
   * @param immobile_dislocations_old Immobile dislocation density from previous timestep
   * @param trial_stress Trial stress from radial return method
   * @param effective_strain_old Effective strain from the previous timestep
   * @param temperature Temperature variable value
   * @param environmental Environmental variable value
   * @param mobile_dislocation_increment Mobile dislocation density incremental change
   * @param immobile_dislocation_increment Immobile dislocation density incremental change
   * @param rom_effective_strain ROM calculated effective strain
   * @param rom_effective_strain_derivative Derivative of ROM calculated effective strain with
   *respect to stress
   */

  void computeROMStrainRate(const Real dt,
                            const Real & mobile_dislocations_old,
                            const Real & immobile_dislocations_old,
                            const ADReal & trial_stress,
                            const Real & effective_strain_old,
                            const ADReal & temperature,
                            const ADReal & environmental,
                            ADReal & mobile_dislocation_increment,
                            ADReal & immobile_dislocation_increment,
                            ADReal & rom_effective_strain,
                            ADReal & rom_effective_strain_derivative);

  /**
   * Function to check input values against applicability windows set by ROM data set
   * @param input Vector of input values
   */
  void checkInputWindows(std::vector<ADReal> & input);

  /**
   * Convert the input variables into the form expected by the ROM Legendre polynomials
   * to have a normalized space from [-1, 1] so that every variable has equal weight
   * @param input Vector of input values
   * @param converted Vector of converted input values
   * @param dconverted Vector of derivative of converted input values with respect to stress
   */
  void convertInput(const std::vector<ADReal> & input,
                    std::vector<std::vector<ADReal>> & converted,
                    std::vector<std::vector<ADReal>> & dconverted);

  /**
   * Assemble the array of Legendre polynomials to be multiplied by the ROM
   * coefficients
   * @param rom_inputs Vector of converted input values
   * @param drom_inputs Vector of derivative of converted input values with respect to stress
   * @param polynomial_inputs Vector of Legendre polynomial transformation
   * @param dpolynomial_inputs Vector of derivative of Legendre polynomial transformation with
   * respect to stress
   */
  void buildPolynomials(const std::vector<std::vector<ADReal>> & rom_inputs,
                        const std::vector<std::vector<ADReal>> & drom_inputs,
                        std::vector<std::vector<std::vector<ADReal>>> & polynomial_inputs,
                        std::vector<std::vector<std::vector<ADReal>>> & dpolynomial_inputs);

  /**
   * Arranges the calculated Legendre polynomials into the order expected by the
   * ROM coefficients and ultiplies the Legendre polynomials by the ROM coefficients to compute the
   * the predicted output values
   * @param coefs Legendre polynomials
   * @param polynomial_inputs Vector of Legendre polynomial transformation
   * @param dpolynomial_inputs Vector of derivative of Legendre polynomial transformation with
   * respect to stress
   * @param rom_outputs Outputs from ROM
   * @param drom_outputs Derivative of outputs from ROM with respect to stress
   */
  void computeValues(const std::vector<std::vector<Real>> & coefs,
                     const std::vector<std::vector<std::vector<ADReal>>> & polynomial_inputs,
                     const std::vector<std::vector<std::vector<ADReal>>> & dpolynomial_inputs,
                     std::vector<ADReal> & rom_outputs,
                     std::vector<ADReal> & drom_outputs);

  /**
   * Computes the output variable increments from the ROM predictions by bringing out of the
   * normalized map to the actual physical values
   * @param dt Timestep size
   * @param old_input_values Previous timestep values of ROM inputs
   * @param rom_outputs Outputs from ROM
   * @param drom_outputs Derivative of outputs from ROM with respect to stress
   * @param input_value_increments Incremental change of input values
   * @param input_value_increments Derivative of the incremental change of input values with respect
   * to stress
   */
  void convertOutput(const Real dt,
                     const std::vector<ADReal> & old_input_values,
                     const std::vector<ADReal> & rom_outputs,
                     const std::vector<ADReal> & drom_outputs,
                     std::vector<ADReal> & input_value_increments,
                     std::vector<ADReal> & dinput_value_increments);

  /**
   * Calculate the value or derivative of Legendre polynomial up to 3rd order
   * @param value Input to Legendre polynomial
   * @param degree Degree of Legendre polynomial
   * @param derivative Flag to return derivative of Legendre polynomial Legendre
   * @return Computed value from Legendre polynomial
   */
  ADReal
  computePolynomial(const ADReal & value, const unsigned int degree, const bool derivative = false);

  /*
   * Calculates and returns the transformed limits for the ROM calculations
   * @return Multi-dimentional vector of transformed limits
   */
  std::vector<std::vector<std::vector<Real>>> getTransformedLimits() const;

  /*
   * Calculates and returns vector utilized in assign values
   * @return Multi-dimentional vector that preallocates calculations for polynomial calculation
   */
  std::vector<std::vector<unsigned int>> getMakeFrameHelper() const;

  /*
   * Returns vector of the functions to use for the conversion of input variables.
   * @return vector of the functions to use for the conversion of input variables.
   */
  virtual std::vector<std::vector<ROMInputTransform>> getTransform() = 0;

  /*
   * Returns factors for the functions for the conversion functions given in getTransform
   * @return factors for the functions for the conversion functions given in getTransform
   */
  virtual std::vector<std::vector<Real>> getTransformCoefs() = 0;

  /* Returns human-readable limits for the inputs. Inputs ordering is
   * 0: mobile_old
   * 1: immobile_old
   * 2: trial stress,
   * 3: effective strain old,
   * 4: temperature
   * 5: environmental factor (optional)
   * @return human-readable limits for the inputs
   */
  virtual std::vector<std::vector<Real>> getInputLimits() = 0;

  /*
   * Material specific coefficients multiplied by the Legendre polynomials for each of the input
   * variables
   * @return Legendre polynomial coefficients
   */
  virtual std::vector<std::vector<Real>> getCoefs() = 0;

  /// Coupled temperature variable
  const ADVariableValue & _temperature;

  /// Optionally coupled environmental factor
  const ADVariableValue & _environmental;

  /// Window applied to input maximum and minimum values
  const Real _window;

  /// Enum to error, warn, or ignore checks that ensure ROM input is within applicability window
  const enum class WindowFailure { ERROR, WARN, IGNORE } _window_failure;

  /// Flag to optinoally allow model extrapolation to zero stress
  const bool _extrapolate_stress;

  /// Flag to output verbose infromation
  const bool _verbose;

  ///@{Material properties for mobile (glissile) dislocation densities (1/m^2)
  ADMaterialProperty(Real) & _mobile_dislocations;
  const MaterialProperty<Real> & _mobile_dislocations_old;
  ///@}

  /// Initial mobile dislocation value
  const Real _initial_mobile_dislocations;

  /// Maximum mobile dislocation increment
  const Real _max_mobile_increment;

  /// Optional mobile dislocation forcing function
  const Function * const _mobile_function;

  /// Container for mobile dislocation increment
  ADReal _mobile_dislocation_increment;

  /// Container for old mobile dislocation value
  Real _mobile_old;

  ///@{Material properties for immobile (locked) dislocation densities (1/m^2)
  ADMaterialProperty(Real) & _immobile_dislocations;
  const MaterialProperty<Real> & _immobile_dislocations_old;
  ///@}

  /// Initial immobile dislocation value
  const Real _initial_immobile_dislocations;

  /// Maximum immobile dislocation increment
  const Real _max_immobile_increment;

  /// Optional immobile dislocation forcing function
  const Function * const _immobile_function;

  /// Container for immobile dislocation increment
  ADReal _immobile_dislocation_increment;

  /// Container for old immobile dislocation value
  Real _immobile_old;

  /// Index corresponding to the position for the stress in the input vector
  const unsigned int _stress_index;

  /// Optional old creep strain forcing function
  const Function * const _creep_strain_old_forcing_function;

  /// Number of inputs for the ROM data set
  unsigned int _num_inputs;

  /// Number of inputs to the ROM data set
  unsigned int _num_outputs;

  /// Legendre polynomial degree for the ROM data set
  unsigned int _degree;

  /// Total number of Legendre polynomial coefficients for the ROM data set
  unsigned int _num_coefs;

  /// Transform rules defined by the ROM data set
  std::vector<std::vector<ROMInputTransform>> _transform;

  /// Transform coefficients defined by the ROM data set
  std::vector<std::vector<Real>> _transform_coefs;

  /// Input limits defined by the ROM data set
  std::vector<std::vector<Real>> _input_limits;

  /// Coefficients used with Legendre polynomials defined by the ROM data set
  std::vector<std::vector<Real>> _coefs;

  /// Flag that checks if environmental factor is included in ROM data set
  bool _use_env;

  /// Limits transformed from readabile input to ROM readable limits
  std::vector<std::vector<std::vector<Real>>> _transformed_limits;

  /// Helper container defined by the ROM data set
  std::vector<std::vector<unsigned int>> _makeframe_helper;

  /// Creep rate material property
  ADMaterialProperty(Real) & _creep_rate;

  /// Material property to indicate if material point is outside of input limits
  MaterialProperty<Real> & _failed;

  /// Container for derivative of creep rate with respect to strain
  ADReal _derivative;

  usingRadialReturnCreepStressUpdateBaseMembers;
};
