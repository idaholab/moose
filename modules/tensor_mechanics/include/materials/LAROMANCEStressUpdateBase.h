//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RadialReturnCreepStressUpdateBase.h"

enum class ROMInputTransform
{
  LINEAR,
  LOG,
  EXP
};

template <bool is_ad>
class LAROMANCEStressUpdateBaseTempl : public RadialReturnCreepStressUpdateBaseTempl<is_ad>
{
  using Material::_dt;
  using Material::_q_point;
  using Material::_qp;
  using Material::_t;
  using Material::coupledGenericValue;
  using RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeResidual;
  using RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeDerivative;

public:
  static InputParameters validParams();

  LAROMANCEStressUpdateBaseTempl(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;

  virtual void initQpStatefulProperties() override;
  virtual void
  computeStressInitialize(const GenericReal<is_ad> & effective_trial_stress,
                          const GenericRankFourTensor<is_ad> & elasticity_tensor) override;
  virtual GenericReal<is_ad> computeResidual(const GenericReal<is_ad> & effective_trial_stress,
                                             const GenericReal<is_ad> & scalar) override;

  virtual GenericReal<is_ad>
  computeDerivative(const GenericReal<is_ad> & /*effective_trial_stress*/,
                    const GenericReal<is_ad> & /*scalar*/) override
  {
    return _derivative;
  }

  virtual void
  computeStressFinalize(const GenericRankTwoTensor<is_ad> & plastic_strain_increment) override;
  virtual GenericReal<is_ad>
  maximumPermissibleValue(const GenericReal<is_ad> & effective_trial_stress) const override;
  virtual Real computeTimeStepLimit() override;

  /// Enum to error, warn, ignore, or extrapolate if input is outside of window of applicability
  enum class WindowFailure
  {
    ERROR,
    WARN,
    IGNORE,
    EXCEPTION,
    EXTRAPOLATE
  };

  /**
   * Precompute the ROM strain rate information for all inputs except for strain. Strain will be
   * computed in the radial return algorithm several times, while the remainder of the inputs remain
   * constant.
   * @param out_index Output index
   */
  void precomputeROM(const unsigned out_index);

  /**
   * Computes the ROM calculated increment for a given output and tile.
   * @param tile Tile index
   * @param out_index Output index
   * @param derivative Optional flag to return derivative of ROM increment with respect to stress.
   * @return ROM computed increment
   */
  GenericReal<is_ad>
  computeROM(const unsigned int tile, const unsigned out_index, const bool derivative = false);

  /**
   * Method to check input values against applicability windows set by ROM data set.
   * @param input Input value
   * @param behavior WindowFailure enum indicating what to do if input is outside of limits
   * @param global_limits Vector of lower and upper global limits of the input
   */
  void checkInputWindow(const GenericReal<is_ad> & input,
                        const WindowFailure behavior,
                        const std::vector<Real> & global_limits);

  /**
   * Convert the input variables into the form expected by the ROM Legendre polynomials to have a
   * normalized space from [-1, 1] so that every variable has equal weight
   * @param input Input value
   * @param transform ROMInputTransform enum indicating how the input is to be transformed
   * @param transform_coef Transform coefficient for the given input
   * @param transformed_limits Transformed limits for the given input
   * @param derivative Optional flag to return derivative of converted input with respect to stress.
   * @return Converted input
   */
  GenericReal<is_ad> normalizeInput(const GenericReal<is_ad> & input,
                                    const ROMInputTransform transform,
                                    const Real transform_coef,
                                    const std::vector<Real> & transformed_limits,
                                    const bool derivative = false);

  /**
   * Assemble the array of Legendre polynomials to be multiplied by the ROM coefficients
   * @param rom_input ROM input
   * @param polynomial_inputs Vector of transformed Legendre polynomials
   * @param drom_input Optional derivative of ROM input with respect to stress
   * @param derivative Optional flag to return derivative of converted input with respect to stress.
   */
  void buildPolynomials(const GenericReal<is_ad> & rom_input,
                        std::vector<GenericReal<is_ad>> & polynomial_inputs,
                        const GenericReal<is_ad> & drom_input = 0,
                        const bool derivative = false);

  /**
   * Arranges the calculated Legendre polynomials into the proper oder and multiplies the Legendre
   * polynomials by the ROM coefficients to compute the predicted output values. This method works
   * with all inputs besides stress, while stress is handled by computeValues
   * @param coefs Legendre polynomial coefficients
   * @param polynomial_inputs Vector of transformed Legendre polynomial
   * @param precomputed Vector that holds the precomputed ROM values
   */
  void precomputeValues(const std::vector<Real> & coefs,
                        const std::vector<std::vector<GenericReal<is_ad>>> & polynomial_inputs,
                        std::vector<GenericReal<is_ad>> & precomputed);

  /**
   * Arranges the calculated Legendre polynomials into the proper oder and multiplies the Legendre
   * polynomials by the ROM coefficients to compute the predicted output values. This method only
   * manipulates the stress input, with all others handled in precomputeValues
   * @param precomputed Precomputed multiplication of polynomials
   * @param polynomial_inputs Vector of Legendre polynomial transformation
   * @param dpolynomial_inputs Vector of derivative of Legendre polynomial transformation with
   * respect to stress
   * @param derivative Optional flag to return derivative of converted computed values with respect
   * to stress.
   * @return ROM output
   */
  GenericReal<is_ad>
  computeValues(const std::vector<GenericReal<is_ad>> & precomputed,
                const std::vector<std::vector<GenericReal<is_ad>>> & polynomial_inputs,
                const std::vector<GenericReal<is_ad>> & dpolynomial_inputs = {},
                const bool derivative = false);

  /**
   * Computes the output variable increments from the ROM predictions by bringing out of the
   * normalized map to the actual physical values
   * @param old_input_values Previous timestep values of ROM inputs
   * @param rom_output Outputs from ROM
   * @param out_index Output index
   * @param drom_output Derivative of output with respect to stress
   * @param derivative Optional flag to return derivative of output with respect to stress.
   * @return Converted ROM output
   */
  GenericReal<is_ad> convertOutput(const std::vector<Real> & old_input_values,
                                   const GenericReal<is_ad> & rom_output,
                                   const unsigned out_index,
                                   const GenericReal<is_ad> & drom_output = 0.0,
                                   const bool derivative = false);

  /**
   * Returns the material specific value for the low bound cutoff of the ROM output,
   * before transformation, and prevents the calculation of a strain value outside
   * the ROM calibration database. Should be overwritten by inheriting classes.
   * @return value of the ROM specific strain calibration bound
   */
  virtual Real romStrainCutoff() = 0;

  /**
   * Calculate the value or derivative of Legendre polynomial up to 3rd order
   * @param value Input to Legendre polynomial
   * @param degree Degree of Legendre polynomial
   * @param derivative Optional flag to return derivative of Legendre polynomial Legendre
   * @return Computed value from Legendre polynomial
   */
  GenericReal<is_ad> computePolynomial(const GenericReal<is_ad> & value,
                                       const unsigned int degree,
                                       const bool derivative = false);

  /**
   * Calculate the sigmoid function weighting for the input based on the limits
   * @param lower Lower limit
   * @param upper Upper limit
   * @param val Input value
   * @param derivative Optional flag to return derivative of the sigmoid w.r.t. the input
   * @return weight
   */
  GenericReal<is_ad> sigmoid(const Real lower,
                             const Real upper,
                             const GenericReal<is_ad> & val,
                             const bool derivative = false);

  /**
   * Compute the weight for applied to each tile based on the location in input-space
   * @param weights Weights for each tile
   * @param input Input value
   * @param in_index Input index
   * @param derivative Optional flag to return derivative of the sigmoid w.r.t. the input
   */
  void computeTileWeight(std::vector<GenericReal<is_ad>> & weights,
                         GenericReal<is_ad> & input,
                         const unsigned int in_index,
                         const bool derivative = false);

  /**
   * Convert input based on the transform type
   * @param x Input value
   * @param transform Enum declaring transform to be performed
   * @param coef Coefficient applied during transformation
   * @param derivative Optional flag to return derivative of the sigmoid w.r.t. the input
   */
  template <typename T>
  void convertValue(T & x,
                    const ROMInputTransform transform,
                    const Real coef,
                    const bool derivative = false)
  {
    if (transform == ROMInputTransform::EXP)
    {
      mooseAssert(coef != 0, "Coefficient must not be zero.");
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
    else if (transform == ROMInputTransform::LINEAR)
    {
      mooseAssert(coef == 0, "Coefficient cannot be supplied with linear transformation");
      if (derivative)
        x = 1.0;
    }
  }

  /*
   * Calculates and returns vector utilized in assign values
   * @return Vector that preallocates indexing calculations for polynomial calculation
   */
  std::vector<unsigned int> getMakeFrameHelper() const;

  /*
   * Calculates and returns the transformed limits for the ROM calculations
   * Indexes are [tile][output][input].
   * Inputs ordering is
   * input[0]: cell_old
   * input[1]: wall_old
   * input[2]: trial stress,
   * input[3]: effective strain old,
   * input[4]: temperature
   * input[5]: environmental factor (optional)
   * output ordering is:
   * output[0]: cell dislocations increment
   * output[1]: wall dislocations increment
   * output[2]: strain increment
   * @param limits Human readable limits
   * @return Multi-dimentional vector of transformed limits
   */
  std::vector<std::vector<std::vector<std::vector<Real>>>>
  getTransformedLimits(const std::vector<std::vector<std::vector<Real>>> limits);

  /*
   * Returns vector of the functions to use for the conversion of input variables.
   * Indexes are [tile][output][input].
   * Inputs ordering is
   * input[0]: cell_old
   * input[1]: wall_old
   * input[2]: trial stress,
   * input[3]: effective strain old,
   * input[4]: temperature
   * input[5]: environmental factor (optional)
   * output ordering is:
   * output[0]: cell dislocations increment
   * output[1]: wall dislocations increment
   * output[2]: strain increment
   * @return vector of the functions to use for the conversion of input variables.
   */
  virtual std::vector<std::vector<std::vector<ROMInputTransform>>> getTransform() = 0;

  /*
   * Returns factors for the functions for the conversion functions given in getTransform
   * Indexes are [tile][output][input].
   * Inputs ordering is
   * input[0]: cell_old
   * input[1]: wall_old
   * input[2]: trial stress,
   * input[3]: effective strain old,
   * input[4]: temperature
   * input[5]: environmental factor (optional)
   * output ordering is:
   * output[0]: cell dislocations increment
   * output[1]: wall dislocations increment
   * output[2]: strain increment
   * @return factors for the functions for the conversion functions given in getTransform
   */
  virtual std::vector<std::vector<std::vector<Real>>> getTransformCoefs() = 0;

  /* Optional method that returns human-readable limits used for normalization. Default is to just
   * use the input limits.
   * Indexes are [tile][input][upper/lower].
   * Inputs ordering is
   * input[0]: cell_old
   * input[1]: wall_old
   * input[2]: trial stress,
   * input[3]: effective strain old,
   * input[4]: temperature
   * input[5]: environmental factor (optional)
   * @return human-readable limits for the normalization limits
   */
  virtual std::vector<std::vector<std::vector<Real>>> getNormalizationLimits()
  {
    return getInputLimits();
  }

  /* Returns human-readable limits for the inputs.
   * Indexes are [tile][input][upper/lower].
   * Inputs ordering is
   * input[0]: cell_old
   * input[1]: wall_old
   * input[2]: trial stress,
   * input[3]: effective strain old,
   * input[4]: temperature
   * input[5]: environmental factor (optional)
   * @return human-readable limits for the input limits
   */
  virtual std::vector<std::vector<std::vector<Real>>> getInputLimits() = 0;

  /*
   * Material specific coefficients multiplied by the Legendre polynomials for each of the input
   * variables
   * @return Legendre polynomial coefficients
   */
  virtual std::vector<std::vector<std::vector<Real>>> getCoefs() = 0;

  /*
   * Material specific orientations of tiling
   * variables
   * @return Vector declaring tiling orientation
   */
  virtual std::vector<unsigned int> getTilings()
  {
    if (_environmental)
      return {1, 1, 1, 1, 1, 1};
    return {1, 1, 1, 1, 1};
  };

  /// Coupled temperature variable
  const GenericVariableValue<is_ad> & _temperature;

  /// Optionally coupled environmental factor
  const GenericMaterialProperty<Real, is_ad> * _environmental;

  /// Vector of WindowFailure enum that informs how to handle input that is outside of the limits
  std::vector<WindowFailure> _window_failure;

  /// Flag to output verbose infromation
  const bool _verbose;

  ///@{Material properties for cell (glissile) dislocation densities (1/m^2)
  GenericMaterialProperty<Real, is_ad> & _cell_dislocations;
  const MaterialProperty<Real> & _cell_dislocations_old;
  ///@}

  /// Initial cell dislocation value
  const Real _initial_cell_dislocations;

  /// Maximum cell dislocation increment
  const Real _max_cell_increment;

  /// Optional cell dislocation forcing function
  const Function * const _cell_function;

  /// Container for cell dislocation increment
  GenericReal<is_ad> _cell_dislocation_increment;

  ///@{Material properties for wall (locked) dislocation densities (1/m^2)
  GenericMaterialProperty<Real, is_ad> & _wall_dislocations;
  const MaterialProperty<Real> & _wall_dislocations_old;
  ///@}

  /// Initial wall dislocation value
  const Real _initial_wall_dislocations;

  /// Maximum wall dislocation increment
  const Real _max_wall_increment;

  /// Optional wall dislocation forcing function
  const Function * const _wall_function;

  /// Container for wall dislocation increment
  GenericReal<is_ad> _wall_dislocation_increment;

  /// Index corresponding to the position for the dislocations with in the cell in the input vector
  const unsigned int _cell_input_index;

  /// Index corresponding to the position for the dislocations within the cell wall in the input vector
  const unsigned int _wall_input_index;

  /// Index corresponding to the position for the stress in the input vector
  const unsigned int _stress_input_index;

  /// Index corresponding to the position for the old strain in the input vector
  const unsigned int _old_strain_input_index;

  /// Index corresponding to the position for the tempeature in the input vector
  const unsigned int _temperature_input_index;

  /// Index corresponding to the position for the environmental factor in the input vector
  const unsigned int _environmental_input_index;

  /// Index corresponding to the position for cell dislocations increment in the output vector
  const unsigned int _cell_output_index;

  /// Index corresponding to the position for wall dislocations increment in the output vector
  const unsigned int _wall_output_index;

  /// Index corresponding to the position for strain increment in the output vector
  const unsigned int _strain_output_index;

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

  /// Normalization limits defined by the ROM data set
  std::vector<std::vector<std::vector<Real>>> _normalization_limits;

  /// Coefficients used with Legendre polynomials defined by the ROM data set
  std::vector<std::vector<std::vector<Real>>> _coefs;

  /// Limits transformed from readabile input to ROM readable limits for normalization
  std::vector<std::vector<std::vector<std::vector<Real>>>> _transformed_normalization_limits;

  /// Helper container defined by the ROM data set
  std::vector<unsigned int> _makeframe_helper;

  /// Creep rate material property
  GenericMaterialProperty<Real, is_ad> & _creep_rate;

  /// Cell dislocations rate of change
  GenericMaterialProperty<Real, is_ad> & _cell_rate;

  /// Wall dislocations rate of change
  GenericMaterialProperty<Real, is_ad> & _wall_rate;

  /// Material property to hold smootherstep applied in order to extrapolate.
  MaterialProperty<Real> & _extrapolation;

  /// Container for derivative of creep increment with respect to strain
  GenericReal<is_ad> _derivative;

  /// Container for input values
  std::vector<GenericReal<is_ad>> _input_values;

  /// Container for old input values
  std::vector<Real> _old_input_values;

  /// Container for converted rom_inputs
  std::vector<std::vector<GenericReal<is_ad>>> _rom_inputs;

  /// Container for ROM polynomial inputs
  std::vector<std::vector<std::vector<GenericReal<is_ad>>>> _polynomial_inputs;

  /// Container for ROM precomputed values
  std::vector<std::vector<GenericReal<is_ad>>> _precomputed_vals;

  /// Container for global limits
  std::vector<std::vector<Real>> _global_limits;

  /// Container for weights for each tile as computed for all input values beside stress
  std::vector<GenericReal<is_ad>> _non_stress_weights;

  /// Container for weights for each tile as computed for all input values beside stress
  std::vector<GenericReal<is_ad>> _weights;

  /// Container for tiling orientations
  std::vector<unsigned int> _tiling;
};

typedef LAROMANCEStressUpdateBaseTempl<false> LAROMANCEStressUpdateBase;
typedef LAROMANCEStressUpdateBaseTempl<true> ADLAROMANCEStressUpdateBase;
