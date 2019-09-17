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

#include "LAROMData.h"

template <ComputeStage compute_stage>
class ADLAROMCreepStressUpdate;

declareADValidParams(ADLAROMCreepStressUpdate);

/**
 * Class to call the Reduced Order Model to predict the behavior of creep
 */

template <ComputeStage compute_stage>
class ADLAROMCreepStressUpdate : public ADRadialReturnCreepStressUpdateBase<compute_stage>
{
public:
  ADLAROMCreepStressUpdate(const InputParameters & parameters);

protected:
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
   * @return Limiting time step
   */
  virtual Real computeTimeStepLimit() override;

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

  /// Function to check input values against applicability windows set by ROM data set
  void checkInputWindows(std::vector<ADReal> & input);

  /* Convert the input variables into the form expected by the ROM Legendre polynomials
   * to have a normalized space from [-1, 1] so that every variable has equal weight
   */
  void convertInput(const std::vector<ADReal> & input,
                    std::vector<std::vector<ADReal>> & converted,
                    std::vector<std::vector<ADReal>> & dconverted);

  /**
   * Assemble the array of Legendre polynomials to be multiplied by the ROM
   * coefficients
   */
  void buildPolynomials(const std::vector<std::vector<ADReal>> & rom_inputs,
                        const std::vector<std::vector<ADReal>> & drom_inputs,
                        std::vector<std::vector<std::vector<ADReal>>> & polynomial_inputs,
                        std::vector<std::vector<std::vector<ADReal>>> & dpolynomial_inputs);

  /**
   * Arranges the calculated Legendre polynomials into the order expected by the
   * ROM coefficients and ultiplies the Legendre polynomials by the ROM coefficients to compute the
   * the predicted output values
   */
  void computeValues(const std::vector<std::vector<Real>> & coefs,
                     const std::vector<std::vector<std::vector<ADReal>>> & polynomial_inputs,
                     const std::vector<std::vector<std::vector<ADReal>>> & dpolynomial_inputs,
                     std::vector<ADReal> & rom_outputs,
                     std::vector<ADReal> & drom_outputs);

  /* Computes the output variable increments from the ROM predictions by bringing out of the
   *normalized map to the actual physical values
   */
  void convertOutput(const Real dt,
                     const std::vector<ADReal> & old_input_values,
                     const std::vector<ADReal> & rom_outputs,
                     const std::vector<ADReal> & drom_outputs,
                     std::vector<ADReal> & input_value_increments,
                     std::vector<ADReal> & dinput_value_increments);

  /// Calculate the value or derivative of Legendre polynomial up to 3rd order
  ADReal
  computePolynomial(const ADReal & value, const unsigned int degree, const bool derivative = false);

private:
  /// Userobject that holds ROM data set
  const LAROMData & _rom;

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

  /// Optional old creep strain forcing function
  const Function * const _creep_strain_old_forcing_function;

  /// Number of inputs for the ROM data set
  const unsigned int _num_inputs;

  /// Number of inputs to the ROM data set
  const unsigned int _num_outputs;

  /// Index corresponding to the position for the stress in the input vector
  const unsigned int _stress_index;

  /// Legendre polynomial degree for the ROM data set
  const unsigned int _degree;

  /// Total number of Legendre polynomial coefficients for the ROM data set
  const unsigned int _num_coefs;

  /// Transform rules defined by the ROM data set
  std::vector<std::vector<unsigned int>> _transform;

  /// Transform coefficients defined by the ROM data set
  std::vector<std::vector<Real>> _transform_coef;

  /// Input limits defined by the ROM data set
  std::vector<std::vector<Real>> _input_limits;

  /// Coefficients used with Legendgre polynomials defined by the ROM data set
  std::vector<std::vector<Real>> _coefs;

  /// Flag that checks if environmental factor is included in ROM data set
  const bool _use_env;

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
