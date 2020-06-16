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

enum class ROMInputTransform
{
  LINEAR,
  LOG,
  EXP,
  INVERSE
};

class ADLAROMANCEStressUpdateBase : public ADRadialReturnCreepStressUpdateBase
{
public:
  static InputParameters validParams();

  ADLAROMANCEStressUpdateBase(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;

  virtual void initQpStatefulProperties() override;
  virtual void computeStressInitialize(const ADReal & effective_trial_stress,
                                       const ADRankFourTensor & elasticity_tensor) override;
  virtual ADReal computeResidual(const ADReal & effective_trial_stress,
                                 const ADReal & scalar) override;

  virtual ADReal computeDerivative(const ADReal & /*effective_trial_stress*/,
                                   const ADReal & /*scalar*/) override
  {
    return _derivative;
  }

  virtual void computeStressFinalize(const ADRankTwoTensor & plastic_strain_increment) override;
  virtual ADReal maximumPermissibleValue(const ADReal & effective_trial_stress) const override;

  /**
   * Compute the limiting value of the time step for this material
   *
   * @return Limiting time step
   */
  virtual Real computeTimeStepLimit() override;

  /// Enum to error, warn, ignore, or extrapolate if input is outside of window of applicability
  enum class WindowFailure
  {
    ERROR,
    WARN,
    IGNORE,
    EXTRAPOLATE
  };

  /**
   * Precompute the ROM strain rate information for all inputs except for strain. Strain will be
   * computed in the radial return algorithm several times, while the remainder of the inputs remain
   * constant.
   * @param out_idx Ouput index
   */
  void precomputeROM(const unsigned out_idx);

  /**
   *Computes the ROM calcualted increment a given output.
   * @param out_idx Ouput index
   * @param derivative Flag to return derivative of ROM increment with respect to stress.
   * @return ROM computed increment
   */
  ADReal computeROM(const unsigned int tile, const unsigned out_idx, const bool derivative = false);

  /**
   * Method to check input values against applicability windows set by ROM data set. In addition,
   * extrapolation is performed if the WindowFailure behavior == extraploation. The returned value
   * is an extrapolated value multiplied by the ROM computed increment if extrapolation occurs.
   * @param input Input value
   * @param limits Vector of lower and upper limits of the input
   * @param behavior WindowFailure MooseEnum indicating what to do if input is outside of limits
   * @param derivative Flag to return derivative of extrapolation with respect to stress.
   * @return Extrapolation value
   */
  void checkInputWindow(const ADReal & input,
                        const WindowFailure behavior,
                        const std::vector<Real> & global_limits);

  /**
   * Convert the input variables into the form expected by the ROM Legendre polynomials
   * to have a normalized space from [-1, 1] so that every variable has equal weight
   * @param input Input value
   * @param transform ROMInputTransform enum indicating how the input is transformed
   * @param transform_coef Transform coefficient for the given input
   * @param transformed_limits Transformed limits for the given input
   * @param derivative Flag to return derivative of converted input with respect to stress.
   * @return Converted input
   */
  ADReal normalizeInput(const ADReal & input,
                        const ROMInputTransform transform,
                        const Real transform_coef,
                        const std::vector<Real> & transformed_limits,
                        const bool derivative = false);

  /**
   * Assemble the array of Legendre polynomials to be multiplied by the ROM coefficients
   * @param rom_input ROM input
   * @param polynomial_inputs Vector of Legendre polynomial transformation
   * @param drom_input Derivative of ROM input with respect to stress
   * @param derivative Flag to return derivative of converted input with respect to stress.
   */
  void buildPolynomials(const ADReal & rom_input,
                        std::vector<ADReal> & polynomial_inputs,
                        const ADReal & drom_input = 0,
                        const bool derivative = false);

  /**
   * Arranges the calculated Legendre polynomials into the order expected by the
   * ROM coefficients and ultiplies the Legendre polynomials by the ROM coefficients to compute the
   * the predicted output values. This method only manipulates all inputs besides stress, with
   * stress handled in computeValues
   * @param coefs Legendre polynomial coefficients
   * @param polynomial_inputs Vector of Legendre polynomial transformation
   * @param precomputed Vector that holds the precomputed ROM values
   */
  void precomputeValues(const std::vector<Real> & coefs,
                        const std::vector<std::vector<ADReal>> & polynomial_inputs,
                        std::vector<ADReal> & precomputed);

  /**
   * Arranges the calculated Legendre polynomials into the order expected by the
   * ROM coefficients and ultiplies the Legendre polynomials by the ROM coefficients to compute the
   * the predicted output values. This method only manipulates the stress input, with all others
   * handled in precomputeValues
   * @param precomputed Precomputed multiplication of polynomials
   * @param polynomial_inputs Vector of Legendre polynomial transformation
   * @param dpolynomial_inputs Vector of derivative of Legendre polynomial transformation with
   * respect to stress
   * @param derivative Flag to return derivative of converted computed values with respect to
   * stress.
   * @return rom_outputs Outputs from ROM
   */
  ADReal computeValues(const std::vector<ADReal> & precomputed,
                       const std::vector<std::vector<ADReal>> & polynomial_inputs,
                       const std::vector<ADReal> & dpolynomial_inputs = {},
                       const bool derivative = false);

  /**
   * Computes the output variable increments from the ROM predictions by bringing out of the
   * normalized map to the actual physical values
   * @param old_input_values Previous timestep values of ROM inputs
   * @param rom_outputs Outputs from ROM
   * @param out_idx Output index
   * @param drom_output Derivative of output with respect to stress
   * @param drom_outputs Derivative of outputs from ROM with respect to stress
   * @param derivative Flag to return derivative of output with respect to stress.
   */
  ADReal convertOutput(const std::vector<Real> & old_input_values,
                       const ADReal & rom_output,
                       const unsigned out_idx,
                       const ADReal & drom_output = 0.0,
                       const bool derivative = false);

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
   * Indexes are [tile][ouput][input].
   * Inputs ordering is
   * input[0]: mobile_old
   * input[1]: immobile_old
   * input[2]: trial stress,
   * input[3]: effective strain old,
   * input[4]: temperature
   * input[5]: environmental factor (optional)
   * output ordering is:
   * output[0]: mobile dislocations increment
   * output[1]: immobile dislocations increment
   * output[2]: strain increment
   * @return Multi-dimentional vector of transformed limits
   */
  std::vector<std::vector<std::vector<std::vector<Real>>>> getTransformedLimits();

  /*
   * Calculates and returns vector utilized in assign values
   * @return Vector that preallocates indexing calculations for polynomial calculation
   */
  std::vector<unsigned int> getMakeFrameHelper() const;

  /*
   * Returns vector of the functions to use for the conversion of input variables.
   * Indexes are [tile][ouput][input].
   * Inputs ordering is
   * input[0]: mobile_old
   * input[1]: immobile_old
   * input[2]: trial stress,
   * input[3]: effective strain old,
   * input[4]: temperature
   * input[5]: environmental factor (optional)
   * output ordering is:
   * output[0]: mobile dislocations increment
   * output[1]: immobile dislocations increment
   * output[2]: strain increment
   * @return vector of the functions to use for the conversion of input variables.
   */
  virtual std::vector<std::vector<std::vector<ROMInputTransform>>> getTransform() = 0;

  /*
   * Returns factors for the functions for the conversion functions given in getTransform
   * Indexes are [tile][ouput][input].
   * Inputs ordering is
   * input[0]: mobile_old
   * input[1]: immobile_old
   * input[2]: trial stress,
   * input[3]: effective strain old,
   * input[4]: temperature
   * input[5]: environmental factor (optional)
   * output ordering is:
   * output[0]: mobile dislocations increment
   * output[1]: immobile dislocations increment
   * output[2]: strain increment
   * @return factors for the functions for the conversion functions given in getTransform
   */
  virtual std::vector<std::vector<std::vector<Real>>> getTransformCoefs() = 0;

  /* Returns human-readable limits for the inputs.
   * Indexes are [tile][ouput][input].
   * Inputs ordering is
   * input[0]: mobile_old
   * input[1]: immobile_old
   * input[2]: trial stress,
   * input[3]: effective strain old,
   * input[4]: temperature
   * input[5]: environmental factor (optional)
   * output ordering is:
   * output[0]: mobile dislocations increment
   * output[1]: immobile dislocations increment
   * output[2]: strain increment
   * @return human-readable limits for the inputs
   */
  virtual std::vector<std::vector<std::vector<Real>>> getInputLimits() = 0;

  /*
   * Material specific coefficients multiplied by the Legendre polynomials for each of the input
   * variables
   * @return Legendre polynomial coefficients
   */
  virtual std::vector<std::vector<std::vector<Real>>> getCoefs() = 0;

  /// Coupled temperature variable
  const ADVariableValue & _temperature;

  /// Optionally coupled environmental factor
  const ADMaterialProperty<Real> * _environmental;

  /// Vector of WindowFailure enum that informs how to handle input that is outside of the limits
  std::vector<WindowFailure> _window_failure;

  /// Flag to output verbose infromation
  const bool _verbose;

  ///@{Material properties for mobile (glissile) dislocation densities (1/m^2)
  ADMaterialProperty<Real> & _mobile_dislocations;
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
  ADMaterialProperty<Real> & _immobile_dislocations;
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

  /// Container for old effective strain
  Real _effective_strain_old;

  /// Index corresponding to the position for the mobile disloations in the input vector
  const unsigned int _mobile_input_idx;

  /// Index corresponding to the position for the immobile disloations in the input vector
  const unsigned int _immobile_input_idx;

  /// Index corresponding to the position for the stress in the input vector
  const unsigned int _stress_input_idx;

  /// Index corresponding to the position for the old strain in the input vector
  const unsigned int _old_strain_input_idx;

  /// Index corresponding to the position for the tempeature in the input vector
  const unsigned int _temperature_input_idx;

  /// Index corresponding to the position for the environmental factor in the input vector
  const unsigned int _environmental_input_idx;

  /// Index corresponding to the position for mobile dislocations increment in the output vector
  const unsigned int _mobile_output_idx;

  /// Index corresponding to the position for immobile dislocations increment in the output vector
  const unsigned int _immobile_output_idx;

  /// Index corresponding to the position for strain increment in the output vector
  const unsigned int _strain_output_idx;

  /// Optional old creep strain forcing function
  const Function * const _creep_strain_old_forcing_function;

  /// Number of ROM tiles
  unsigned int _num_tiles;

  /// Number of inputs for the ROM data set
  unsigned int _num_inputs;

  /// Number of inputs to the ROM data set
  unsigned int _num_outputs;

  /// Legendre polynomial degree for the ROM data set
  unsigned int _degree;

  /// Total number of Legendre polynomial coefficients for the ROM data set
  unsigned int _num_coefs;

  /// Transform rules defined by the ROM data set
  std::vector<std::vector<std::vector<ROMInputTransform>>> _transform;

  /// Transform coefficients defined by the ROM data set
  std::vector<std::vector<std::vector<Real>>> _transform_coefs;

  /// Input limits defined by the ROM data set
  std::vector<std::vector<std::vector<Real>>> _input_limits;

  /// Coefficients used with Legendre polynomials defined by the ROM data set
  std::vector<std::vector<std::vector<Real>>> _coefs;

  /// Limits transformed from readabile input to ROM readable limits
  std::vector<std::vector<std::vector<std::vector<Real>>>> _transformed_limits;

  /// Helper container defined by the ROM data set
  std::vector<unsigned int> _makeframe_helper;

  /// Creep rate material property
  ADMaterialProperty<Real> & _creep_rate;

  /// Mobile dislocations rate
  ADMaterialProperty<Real> & _mobile_rate;

  /// Immobile dislocations rate
  ADMaterialProperty<Real> & _immobile_rate;

  /// Material property to hold smootherstep applied in order to extrapolate.
  ADMaterialProperty<Real> & _extrapolation;

  /// Container for derivative of creep rate with respect to strain
  ADReal _derivative;

  /// Container for input values
  std::vector<ADReal> _input_values;

  /// Container for old input values
  std::vector<Real> _old_input_values;

  /// Container for converted rom_inputs
  std::vector<std::vector<ADReal>> _rom_inputs;

  /// Container for ROM polynomial inputs
  std::vector<std::vector<std::vector<ADReal>>> _polynomial_inputs;

  /// Container for ROM precomputed values
  std::vector<std::vector<ADReal>> _precomputed_vals;

  ADReal
  sigmoid(const Real lower, const Real upper, const ADReal & val, const bool derivative = false);
  void computeTileWeight(std::vector<ADReal> & weights,
                         ADReal & input,
                         const unsigned int in_idx,
                         const bool derivative = false);

  std::vector<std::vector<Real>> _global_limits;
  std::vector<ADReal> _non_stress_weights;
  std::vector<ADReal> _weights;
  std::vector<unsigned int> _tiling;

  virtual std::vector<unsigned int> getTilings()
  {
    if (_environmental)
      return {1, 1, 1, 1, 1, 1};
    return {1, 1, 1, 1, 1};
  };

  template <typename T>
  void convertValue(T & x,
                    const ROMInputTransform transform,
                    const Real coef,
                    const bool derivative = false)
  {
    if (transform == ROMInputTransform::EXP)
    {
      if (derivative)
        x = std::exp(x / coef) / coef;
      else
        x = std::exp(x / coef);
    }
    else if (transform == ROMInputTransform::LOG)
    {
      mooseAssert(x + coef > 0, "Sum must be greater than 0.");
      if (derivative)
        x = 1.0 / (x + coef);
      else
        x = std::log(x + coef);
    }
    else if (transform == ROMInputTransform::INVERSE)
    {
      mooseAssert(input + transform_coef != 0, "Sum must not equal zero.");
      if (derivative)
        x = -1.0 / Utility::pow<2>(x + coef);
      else
        x = 1.0 / (x + coef);
    }
    else if (derivative)
      x = 1.0;
  }
};
