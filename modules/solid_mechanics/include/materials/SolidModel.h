#ifndef SOLIDMODEL_H
#define SOLIDMODEL_H

#include "Material.h"
#include "SymmTensor.h"

// Forward declarations
class ConstitutiveModel;
class SolidModel;
class SymmElasticityTensor;
class VolumetricModel;
namespace SolidMechanics
{
class Element;
}


template<>
InputParameters validParams<SolidModel>();

/**
 * SolidModel is the base class for all this module's solid mechanics material models.
 */
class SolidModel : public Material
{
public:
  SolidModel( const std::string & name,
              InputParameters parameters );
  virtual ~SolidModel();

  virtual void initStatefulProperties( unsigned n_points );

  virtual void applyThermalStrain();
  virtual void applyVolumetricStrain();

  static void rotateSymmetricTensor( const ColumnMajorMatrix & R, const SymmTensor & T,
                                     SymmTensor & result );

  enum CRACKING_RELEASE
  {
    CR_ABRUPT = 0,
    CR_EXPONENTIAL,
    CR_POWER,
    CR_UNKNOWN
  };

  QBase * qrule()
  {
    return _qrule;
  }
  const Point & q_point( unsigned i ) const
  {
    return _q_point[i];
  }
  Real JxW( unsigned i ) const
  {
    return _JxW[i];
  }

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

  Function * const _youngs_modulus_function;
  Function * const _poissons_ratio_function;

  const CRACKING_RELEASE _cracking_release;
  const Real _cracking_stress;
  const Real _cracking_residual_stress;
  Real _cracking_alpha;
  std::vector<unsigned int> _active_crack_planes;
  const unsigned int _max_cracks;
  const Real _cracking_neg_fraction;
  // std::map<Point, unsigned> _cracked_this_step_count;
  // std::map<Point, unsigned> _cracked_this_step;

  const bool _has_temp;
  VariableValue & _temperature;
  VariableValue & _temperature_old;
  const Real _alpha;
  Function * const _alpha_function;
  bool _has_stress_free_temp;
  Real _stress_free_temp;

  std::map<SubdomainID, std::vector<VolumetricModel*> > _volumetric_models;
  std::vector<MaterialProperty<Real>*> _volumetric_strain;
  std::vector<MaterialProperty<Real>*> _volumetric_strain_old;

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
  MaterialProperty<SymmElasticityTensor> & _Jacobian_mult;

  // Accumulate derivatives of strain tensors with respect to Temperature into this
  SymmTensor _d_strain_dT;

  // The derivative of the stress with respect to Temperature
  MaterialProperty<SymmTensor> & _d_stress_dT;

  SymmTensor _total_strain_increment;
  SymmTensor _strain_increment;

  MaterialProperty<Real> & _SED;
  MaterialProperty<Real> & _SED_old;
  const bool _compute_JIntegral;
  MaterialProperty<ColumnMajorMatrix> & _Eshelby_tensor;

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

  virtual void computeEshelby();
  virtual void computeStrainEnergyDensity();

  /*
   * Determine whether new cracks have formed.
   * Rotate old and new stress to global, if cracking active
   */
  virtual void crackingStressRotation();

  virtual Real computeCrackFactor( int i, Real & sigma, Real & flagVal );

  /// Rotate stress to current configuration
  virtual void finalizeStress();


  virtual void computePreconditioning();

  void applyCracksToTensor( SymmTensor & tensor, const RealVectorValue & sigma );

  void
  elasticityTensor( SymmElasticityTensor * e );

  SymmElasticityTensor *
  elasticityTensor() const
  {
    return _local_elasticity_tensor;
  }

  const SolidMechanics::Element * element() const
  {
    return _element;
  }

  int delta(int i, int j) const
  {
    return i == j;
  }

  template<typename T>
  MaterialProperty<T> & createProperty(const std::string & prop_name)
  {
    std::string name(prop_name + _appended_property_name);
    return declareProperty<T>(name);
  }

  template<typename T>
  MaterialProperty<T> & createPropertyOld(const std::string & prop_name)
  {
    std::string name(prop_name + _appended_property_name);
    return declarePropertyOld<T>(name);
  }

  virtual void checkElasticConstants();

  virtual void createElasticityTensor();

  std::vector<SubdomainID> _block_id;

  std::map<SubdomainID, ConstitutiveModel*> _constitutive_model;
  // This set keeps track of the dynamic memory allocated in this object
  std::set<ConstitutiveModel *> _models_to_free;
  bool _constitutive_active;

  /// Compute the stress (sigma += deltaSigma)
  virtual void computeConstitutiveModelStress();

  void createConstitutiveModel(const std::string & cm_name, const InputParameters & params);


private:

  void computeCrackStrainAndOrientation( ColumnMajorMatrix & principal_strain );

  SolidMechanics::Element * createElement( const std::string & name,
                                           InputParameters & parameters );

  SolidMechanics::Element * _element;

  SymmElasticityTensor * _local_elasticity_tensor;

};



#endif
