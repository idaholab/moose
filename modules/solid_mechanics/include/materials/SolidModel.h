/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SOLIDMODEL_H
#define SOLIDMODEL_H

#include "DerivativeMaterialInterface.h"
#include "SymmTensor.h"

// Forward declarations
class ConstitutiveModel;
class SolidModel;
class SymmElasticityTensor;
class PiecewiseLinear;
class VolumetricModel;
namespace SolidMechanics
{
class Element;
}

template <>
InputParameters validParams<SolidModel>();

/**
 * SolidModel is the base class for all this module's solid mechanics material models.
 */
class SolidModel : public DerivativeMaterialInterface<Material>
{
public:
  SolidModel(const InputParameters & parameters);
  virtual ~SolidModel();

  virtual void initStatefulProperties(unsigned n_points);

  virtual void applyThermalStrain();
  virtual void applyVolumetricStrain();

  static void
  rotateSymmetricTensor(const ColumnMajorMatrix & R, const SymmTensor & T, SymmTensor & result);

  enum CRACKING_RELEASE
  {
    CR_ABRUPT = 0,
    CR_EXPONENTIAL,
    CR_POWER,
    CR_UNKNOWN
  };

  QBase * qrule() { return _qrule; }
  const Point & q_point(unsigned i) const { return _q_point[i]; }
  Real JxW(unsigned i) const { return _JxW[i]; }

protected:
  Moose::CoordinateSystemType _coord_type;

  const std::string _appended_property_name;

  bool _bulk_modulus_set;
  bool _lambda_set;
  bool _poissons_ratio_set;
  bool _shear_modulus_set;
  bool _youngs_modulus_set;

  Real _bulk_modulus;
  Real _lambda;
  Real _poissons_ratio;
  Real _shear_modulus;
  Real _youngs_modulus;

  Function * _youngs_modulus_function;
  Function * _poissons_ratio_function;

  const CRACKING_RELEASE _cracking_release;
  Real _cracking_stress;
  const Real _cracking_residual_stress;
  const Real _cracking_beta;
  const std::string _compute_method;
  Function * const _cracking_stress_function;

  Real _cracking_alpha;
  std::vector<unsigned int> _active_crack_planes;
  const unsigned int _max_cracks;
  const Real _cracking_neg_fraction;
  // std::map<Point, unsigned> _cracked_this_step_count;
  // std::map<Point, unsigned> _cracked_this_step;

  const bool _has_temp;
  const VariableValue & _temperature;
  const VariableValue & _temperature_old;
  const VariableGradient & _temp_grad;
  const Real _alpha;
  Function * _alpha_function;
  PiecewiseLinear * _piecewise_linear_alpha_function;
  bool _has_stress_free_temp;
  Real _stress_free_temp;
  bool _mean_alpha_function;
  Real _ref_temp;

  std::map<SubdomainID, std::vector<MooseSharedPointer<VolumetricModel>>> _volumetric_models;
  std::set<std::string> _dep_matl_props;

  MaterialProperty<SymmTensor> & _stress;

private:
  MaterialProperty<SymmTensor> & _stress_old_prop;

protected:
  SymmTensor _stress_old;

  MaterialProperty<SymmTensor> & _total_strain;
  MaterialProperty<SymmTensor> & _total_strain_old;

  MaterialProperty<SymmTensor> & _elastic_strain;
  MaterialProperty<SymmTensor> & _elastic_strain_old;

  MaterialProperty<RealVectorValue> * _crack_flags;
  MaterialProperty<RealVectorValue> * _crack_flags_old;
  RealVectorValue _crack_flags_local;
  MaterialProperty<RealVectorValue> * _crack_count;
  MaterialProperty<RealVectorValue> * _crack_count_old;
  MaterialProperty<ColumnMajorMatrix> * _crack_rotation;
  MaterialProperty<ColumnMajorMatrix> * _crack_rotation_old;
  MaterialProperty<RealVectorValue> * _crack_strain;
  MaterialProperty<RealVectorValue> * _crack_strain_old;
  MaterialProperty<RealVectorValue> * _crack_max_strain;
  MaterialProperty<RealVectorValue> * _crack_max_strain_old;
  ColumnMajorMatrix _principal_strain;

  MaterialProperty<SymmElasticityTensor> & _elasticity_tensor;
  MaterialProperty<SymmElasticityTensor> & _elasticity_tensor_old;
  MaterialProperty<SymmElasticityTensor> & _Jacobian_mult;

  // Accumulate derivatives of strain tensors with respect to Temperature into this
  SymmTensor _d_strain_dT;

  // The derivative of the stress with respect to Temperature
  MaterialProperty<SymmTensor> & _d_stress_dT;

  SymmTensor _total_strain_increment;
  SymmTensor _strain_increment;

  const bool _compute_JIntegral;
  const bool _compute_InteractionIntegral;
  bool _store_stress_older;

  // These are used in calculation of the J integral
  MaterialProperty<Real> * _SED;
  MaterialProperty<Real> * _SED_old;
  MaterialProperty<ColumnMajorMatrix> * _Eshelby_tensor;
  MaterialProperty<RealVectorValue> * _J_thermal_term_vec;

  // This is used in calculation of the J Integral and Interaction Integral
  MaterialProperty<Real> * _current_instantaneous_thermal_expansion_coef;

  virtual void initQpStatefulProperties();

  virtual void initialSetup();
  virtual void timestepSetup();
  virtual void jacobianSetup();

  virtual void computeProperties();

  void computeElasticityTensor();
  /**
   * Return true if the elasticity tensor changed.
   */
  virtual bool updateElasticityTensor(SymmElasticityTensor & tensor);

  virtual void elementInit() {}

  /// Modify increment for things like thermal strain
  virtual void modifyStrainIncrement();

  /// Determine cracking directions.  Rotate elasticity tensor.
  virtual void crackingStrainDirections();

  virtual unsigned int getNumKnownCrackDirs() const;

  /// Compute the stress (sigma += deltaSigma)
  virtual void computeStress()
  {
    mooseError("SolidModel::computeStress must be defined by the derived class");
  }

  // Compute Eshelby tensor, used in J Integral calculation
  virtual void computeEshelby();

  // Compute strain energy density, used in Eshelby tensor calculation
  virtual void computeStrainEnergyDensity();

  // Compute quantity used in thermal term of J Integral
  virtual void computeThermalJvec();

  // Compute current thermal expansion coefficient, used in J Integral and Interaction Integral
  virtual void computeCurrentInstantaneousThermalExpansionCoefficient();

  /*
   * Determine whether new cracks have formed.
   * Rotate old and new stress to global, if cracking active
   */
  virtual void crackingStressRotation();

  virtual Real computeCrackFactor(int i, Real & sigma, Real & flagVal);

  /// Rotate stress to current configuration
  virtual void finalizeStress();

  virtual void computePreconditioning();

  void applyCracksToTensor(SymmTensor & tensor, const RealVectorValue & sigma);

  void elasticityTensor(SymmElasticityTensor * e);

  SymmElasticityTensor * elasticityTensor() const { return _local_elasticity_tensor; }

  const SolidMechanics::Element * element() const { return _element; }

  int delta(int i, int j) const { return i == j; }

  template <typename T>
  MaterialProperty<T> & createProperty(const std::string & prop_name)
  {
    std::string name(prop_name + _appended_property_name);
    return declareProperty<T>(name);
  }

  template <typename T>
  MaterialProperty<T> & createPropertyOld(const std::string & prop_name)
  {
    std::string name(prop_name + _appended_property_name);
    return declarePropertyOld<T>(name);
  }

  virtual void checkElasticConstants();

  virtual void createElasticityTensor();

  std::vector<SubdomainID> _block_id;

  std::map<SubdomainID, MooseSharedPointer<ConstitutiveModel>> _constitutive_model;
  // This set keeps track of the dynamic memory allocated in this object
  std::set<MooseSharedPointer<ConstitutiveModel>> _models_to_free;
  bool _constitutive_active;

  /// Compute the stress (sigma += deltaSigma)
  virtual void computeConstitutiveModelStress();

  void createConstitutiveModel(const std::string & cm_name);

  ///@{ Restartable data to check for the zeroth and first time steps for thermal calculations
  bool & _step_zero;
  bool & _step_one;
  ///@}

private:
  void computeCrackStrainAndOrientation(ColumnMajorMatrix & principal_strain);

  SolidMechanics::Element * createElement();

  SolidMechanics::Element * _element;

  SymmElasticityTensor * _local_elasticity_tensor;
};

#endif
