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
#include "nlohmann/json.h"

template <bool is_ad>
class LAROMANCEStressUpdateBaseTempl : public RadialReturnCreepStressUpdateBaseTempl<is_ad>
{

public:
  static InputParameters validParams();

  LAROMANCEStressUpdateBaseTempl(const InputParameters & parameters);

  virtual void resetIncrementalMaterialProperties() override;
  virtual void
  storeIncrementalMaterialProperties(const unsigned int total_number_substeps) override;

protected:
  virtual void exportJSON();

  virtual bool substeppingCapabilityEnabled() override;

  enum class ROMInputTransform
  {
    LINEAR,
    LOG,
    EXP
  };

  virtual void initialSetup() override;

  // Setup unit conversion factors. The required units in the ROM are:
  // Cell dislocation density: m^-2
  // Wall dislocation density: m^-2
  // MX phase fracture: nondim.
  // stress: MPa
  // strain: nondim.
  // temperature: K
  virtual void setupUnitConversionFactors(const InputParameters & parameters);

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

  void outputIterationSummary(std::stringstream * iter_output,
                              const unsigned int total_it) override;

  virtual void outputIterationStep(std::stringstream * iter_output,
                                   const GenericReal<is_ad> & effective_trial_stress,
                                   const GenericReal<is_ad> & scalar,
                                   const Real reference_residual) override;

  /// Enum to error, warn, ignore, or extrapolate if input is outside of window of applicability
  enum class WindowFailure
  {
    ERROR,
    EXCEPTION,
    WARN,
    IGNORE,
    DONOTHING,
    USELIMIT,
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
   * @param partition Partition index
   * @param out_index Output index
   * @param derivative Optional flag to return derivative of ROM increment with respect to stress.
   * @return ROM computed increment
   */
  GenericReal<is_ad> computeROM(const unsigned int tile,
                                const unsigned int partition,
                                const unsigned out_index,
                                const bool derivative = false);

  /**
   * Checks if the input combination is in a specific tile
   * @param p Partition index
   * @param t Tile index
   * @return bool if in tile
   */
  bool checkInTile(const unsigned int p, const unsigned int t) const;

  /**
   * Checks if two tile domains are equal
   * @param p Partition index
   * @param t Tile 1 index
   * @param tt Tile 2 index
   * @param in_index input index
   * @return integer 0 (false) or 1 (true)
   */
  bool areTilesNotIdentical(const unsigned int p,
                            const unsigned int t,
                            const unsigned int tt,
                            const unsigned int in_index);

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
   * @param p Partition index
   * @param rom_input ROM input
   * @param polynomial_inputs Vector of transformed Legendre polynomials
   * @param drom_input Optional derivative of ROM input with respect to stress
   * @param derivative Optional flag to return derivative of converted input with respect to stress.
   */
  void buildPolynomials(const unsigned int p,
                        const GenericReal<is_ad> & rom_input,
                        std::vector<GenericReal<is_ad>> & polynomial_inputs,
                        const GenericReal<is_ad> & drom_input = 0,
                        const bool derivative = false);

  /**
   * Arranges the calculated Legendre polynomials into the proper oder and multiplies the Legendre
   * polynomials by the ROM coefficients to compute the predicted output values. This method works
   * with all inputs besides stress, while stress is handled by computeValues
   * @param p Partition index
   * @param coefs Legendre polynomial coefficients
   * @param polynomial_inputs Vector of transformed Legendre polynomial
   * @param precomputed Vector that holds the precomputed ROM values
   */
  void precomputeValues(const unsigned int p,
                        const std::vector<Real> & coefs,
                        const std::vector<std::vector<GenericReal<is_ad>>> & polynomial_inputs,
                        std::vector<GenericReal<is_ad>> & precomputed);

  /**
   * Arranges the calculated Legendre polynomials into the proper oder and multiplies the Legendre
   * polynomials by the ROM coefficients to compute the predicted output values. This method only
   * manipulates the stress input, with all others handled in precomputeValues
   * @param p Partition index
   * @param precomputed Precomputed multiplication of polynomials
   * @param polynomial_inputs Vector of Legendre polynomial transformation
   * @param dpolynomial_inputs Vector of derivative of Legendre polynomial transformation with
   * respect to stress
   * @param derivative Optional flag to return derivative of converted computed values with respect
   * to stress.
   * @return ROM output
   */
  GenericReal<is_ad>
  computeValues(const unsigned int p,
                const std::vector<GenericReal<is_ad>> & precomputed,
                const std::vector<std::vector<GenericReal<is_ad>>> & polynomial_inputs,
                const std::vector<GenericReal<is_ad>> & dpolynomial_inputs = {},
                const bool derivative = false);

  /**
   * Computes the output variable increments from the ROM predictions by bringing out of the
   * normalized map to the actual physical values
   * @param p Partition index
   * @param old_input_values Previous timestep values of ROM inputs
   * @param rom_output Outputs from ROM
   * @param out_index Output index
   * @param drom_output Derivative of output with respect to stress
   * @param derivative Optional flag to return derivative of output with respect to stress.
   * @return Converted ROM output
   */
  virtual GenericReal<is_ad> convertOutput(const unsigned int p,
                                           const std::vector<Real> & old_input_values,
                                           const GenericReal<is_ad> & rom_output,
                                           const unsigned out_index,
                                           const GenericReal<is_ad> & drom_output = 0.0,
                                           const bool derivative = false);

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
   * Compute the contribution (weight) of each tile in each partition,
   * based on the input and tile boundaries (in terms of input domain).
   * @param weights Weights for each tile
   * @param input Input value
   * @param in_index Input index
   * @param derivative Optional flag to return derivative of the sigmoid w.r.t. the input
   */
  void computeTileWeight(std::vector<std::vector<GenericReal<is_ad>>> & weights,
                         GenericReal<is_ad> & input,
                         const unsigned int in_index,
                         const bool derivative = false);

  /**
   * Compute the weight of the different partitions
   * @param weights Weights for each partition
   * @param derivative Optional flag to return derivative of the sigmoid w.r.t. the input
   */
  virtual void computePartitionWeights(std::vector<GenericReal<is_ad>> & weights,
                                       std::vector<GenericReal<is_ad>> & dweights_dstress)
  {
    if (_num_partitions != 1)
      mooseError("Internal error: If number of partitions is not one, then computePartitionWeights "
                 "must be defined");
    weights[0] = 1.0;
    dweights_dstress[0] = 0.0;
  }

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
      mooseAssert(coef == 0.0, "Coefficient cannot be supplied with linear transformation");
      if (derivative)
        x = 1.0;
    }
  }

  /*
   * Calculates and returns vector utilized in assign values
   * @param p Partition index
   * @return Vector that preallocates indexing calculations for polynomial calculation
   */
  std::vector<unsigned int> getMakeFrameHelper(const unsigned int p) const;

  /*
   * Calculates and returns the transformed limits for the ROM calculations
   * Indexes are [partition][tile][output][input].
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
   * @param p Partition index
   * @param limits Human readable limits
   * @return Multi-dimentional vector of transformed limits
   */
  std::vector<std::vector<std::vector<std::vector<Real>>>>
  getTransformedLimits(const unsigned int p,
                       const std::vector<std::vector<std::vector<Real>>> limits);

  /*
   * Returns vector of the functions to use for the conversion of input variables.
   * Indexes are [partition][tile][output][input].
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
  virtual std::vector<std::vector<std::vector<std::vector<ROMInputTransform>>>> getTransform()
  {
    checkJSONKey("transform");
    return _json["transform"]
        .template get<std::vector<std::vector<std::vector<std::vector<ROMInputTransform>>>>>();
  }

  /*
   * Returns factors for the functions for the conversion functions given in getTransform
   * Indexes are [partition][tile][output][input].
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
  virtual std::vector<std::vector<std::vector<std::vector<Real>>>> getTransformCoefs()
  {
    checkJSONKey("transform_coefs");
    return _json["transform_coefs"]
        .template get<std::vector<std::vector<std::vector<std::vector<Real>>>>>();
  }

  /* Optional method that returns human-readable limits used for normalization. Default is to just
   * use the input limits.
   * Indexes are [partition][tile][input][upper/lower].
   * Inputs ordering is
   * input[0]: cell_old
   * input[1]: wall_old
   * input[2]: trial stress,
   * input[3]: effective strain old,
   * input[4]: temperature
   * input[5]: environmental factor (optional)
   * @return human-readable limits for the normalization limits
   */
  virtual std::vector<std::vector<std::vector<std::vector<Real>>>> getNormalizationLimits()
  {
    if (_json.contains("normalization_limits"))
      return _json["normalization_limits"]
          .template get<std::vector<std::vector<std::vector<std::vector<Real>>>>>();

    return getInputLimits();
  }

  /* Returns human-readable limits for the inputs.
   * Indexes are [partition][tile][input][upper/lower].
   * Inputs ordering is
   * input[0]: cell_old
   * input[1]: wall_old
   * input[2]: trial stress,
   * input[3]: effective strain old,
   * input[4]: temperature
   * input[5]: environmental factor (optional)
   * @return human-readable limits for the input limits
   */
  virtual std::vector<std::vector<std::vector<std::vector<Real>>>> getInputLimits()
  {
    checkJSONKey("input_limits");
    return _json["input_limits"]
        .template get<std::vector<std::vector<std::vector<std::vector<Real>>>>>();
  }

  /*
   * Material specific coefficients multiplied by the Legendre polynomials for each of the input
   * variables
   * @return Legendre polynomial coefficients
   */
  virtual std::vector<std::vector<std::vector<std::vector<Real>>>> getCoefs()
  {
    checkJSONKey("coefs");
    return _json["coefs"].template get<std::vector<std::vector<std::vector<std::vector<Real>>>>>();
  }

  /*
   * Material specific orientations of tiling
   * variables. Indexing is partition, then input
   * @return Vector of a vector declaring tiling orientation
   */
  virtual std::vector<std::vector<unsigned int>> getTilings()
  {
    if (_json.contains("tiling"))
      return _json["tiling"].template get<std::vector<std::vector<unsigned int>>>();

    if (_environmental)
      return {{1, 1, 1, 1, 1, 1}};
    return {{1, 1, 1, 1, 1}};
  };

  /*
   * Minimum strain value allowed by the ROM. This is material specific, and needs to be overwritten
   * by individual roms and each partition
   * @return Vector of material specific ROM low strain value for each partition
   */
  virtual std::vector<Real> getStrainCutoff()
  {
    checkJSONKey("cutoff");
    return _json["cutoff"].template get<std::vector<Real>>();
  }

  /// Coupled temperature variable
  const GenericVariableValue<is_ad> & _temperature;

  /// Optionally coupled environmental factor
  const GenericMaterialProperty<Real, is_ad> * _environmental;

  /*
   * Vector of vectors WindowFailure enum that informs how to handle input that is outside of the
   * limits. Shape is number of inputs by 2 (lower and upper window enum)
   */
  std::vector<std::pair<WindowFailure, WindowFailure>> _window_failure;

  /// Flag to output verbose infromation
  const bool _verbose;

  ///@{Material properties for cell (glissile) dislocation densities (1/m^2)
  GenericMaterialProperty<Real, is_ad> & _cell_dislocations;
  const MaterialProperty<Real> & _cell_dislocations_old;
  ///@}

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

  /// Maximum wall dislocation increment
  const Real _max_wall_increment;

  /// Optional wall dislocation forcing function
  const Function * const _wall_function;

  /// Optiontal effective stress forcing function
  const Function * const _stress_function;

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

  /// Number of partitions
  unsigned int _num_partitions;

  /// Number of ROM tiles per partition
  std::vector<unsigned int> _num_tiles;

  /// Number of inputs for the ROM data set
  unsigned int _num_inputs;

  /// Number of inputs to the ROM data set
  unsigned int _num_outputs;

  /// Legendre polynomial degree for the ROM data set for each partition
  std::vector<unsigned int> _degree;

  /// Total number of Legendre polynomial coefficients for the ROM data set in each parition
  std::vector<unsigned int> _num_coefs;

  /// Transform rules defined by the ROM data set for each partition
  std::vector<std::vector<std::vector<std::vector<ROMInputTransform>>>> _transform;

  /// Transform coefficients defined by the ROM data set for each partition
  std::vector<std::vector<std::vector<std::vector<Real>>>> _transform_coefs;

  /// Input limits defined by the ROM data set for each partition
  std::vector<std::vector<std::vector<std::vector<Real>>>> _input_limits;

  /// Normalization limits defined by the ROM data set for each partition
  std::vector<std::vector<std::vector<std::vector<Real>>>> _normalization_limits;

  /// Coefficients used with Legendre polynomials defined by the ROM data set for each partition
  std::vector<std::vector<std::vector<std::vector<Real>>>> _coefs;

  /// Limits transformed from readabile input to ROM readable limits for normalization
  std::vector<std::vector<std::vector<std::vector<std::vector<Real>>>>>
      _transformed_normalization_limits;

  /// Helper container defined by the ROM data set
  std::vector<std::vector<unsigned int>> _makeframe_helper;

  /// Creep rate material property
  GenericMaterialProperty<Real, is_ad> & _creep_rate;

  /// Cell dislocations rate of change
  GenericMaterialProperty<Real, is_ad> & _cell_rate;

  /// Wall dislocations rate of change
  GenericMaterialProperty<Real, is_ad> & _wall_rate;

  /// Material property to hold smootherstep applied in order to extrapolate.
  MaterialProperty<Real> & _extrapolation;

  /// Material property to store partition weight.
  GenericMaterialProperty<Real, is_ad> & _second_partition_weight;

  /// Container for derivative of creep increment with respect to strain
  GenericReal<is_ad> _derivative;

  /// Container for input values
  std::vector<GenericReal<is_ad>> _input_values;

  /// Container for old input values
  std::vector<Real> _old_input_values;

  /// Container for converted rom_inputs
  std::vector<std::vector<std::vector<GenericReal<is_ad>>>> _rom_inputs;

  /// Container for ROM polynomial inputs
  std::vector<std::vector<std::vector<std::vector<GenericReal<is_ad>>>>> _polynomial_inputs;

  /// Container for ROM precomputed values
  std::vector<std::vector<std::vector<GenericReal<is_ad>>>> _precomputed_vals;

  /// Container for global limits
  std::vector<std::pair<Real, Real>> _global_limits;

  /// Container for weights for each tile as computed for all input values beside stress
  std::vector<std::vector<GenericReal<is_ad>>> _non_stress_weights;

  /// Container for weights for each tile as computed for all input values beside stress
  std::vector<std::vector<GenericReal<is_ad>>> _weights;

  /// Container for weights for each tile as computed for all input values beside stress
  std::vector<GenericReal<is_ad>> _partition_weights;

  /// Container for d_parition_weights d_stress
  std::vector<GenericReal<is_ad>> _dpartition_weight_dstress;

  /// Container for tiling orientations
  std::vector<std::vector<unsigned int>> _tiling;

  /// Container for strain cutoff
  std::vector<Real> _cutoff;

  /// Unit conversion factors required to convert from the specified unit to MPa
  Real _stress_ucf;

  ///@{Material properties accumulated at substeps
  GenericMaterialProperty<Real, is_ad> & _wall_dislocations_step;
  GenericMaterialProperty<Real, is_ad> & _cell_dislocations_step;
  ///@}

  /// Total plastic strain increment in step (summing substep contributions)
  RankTwoTensor _plastic_strain_increment;

  /// Material property capturing number of substeps for output purposes (defaults to one if substepping isn't used)
  MaterialProperty<Real> & _number_of_substeps;

  /// index names for error output
  std::vector<std::string> _index_name;

  /// check if a JSON file was loaded and if the specified key exists
  void checkJSONKey(const std::string & key);

  /// JSON object constructed from the datafile
  nlohmann::json _json;

  using Material::_dt;
  using Material::_name;
  using Material::_q_point;
  using Material::_qp;
  using Material::_t;
  using Material::coupledGenericValue;
  using RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeResidual;
  using RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeDerivative;
  using RadialReturnCreepStressUpdateBaseTempl<is_ad>::_apply_strain;
  using RadialReturnCreepStressUpdateBaseTempl<is_ad>::initQpStatefulProperties;
  using RadialReturnCreepStressUpdateBaseTempl<is_ad>::outputIterationStep;
  using RadialReturnCreepStressUpdateBaseTempl<is_ad>::outputIterationSummary;
};

typedef LAROMANCEStressUpdateBaseTempl<false> LAROMANCEStressUpdateBase;
typedef LAROMANCEStressUpdateBaseTempl<true> ADLAROMANCEStressUpdateBase;
