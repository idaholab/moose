#ifndef MATERIALMODEL_H
#define MATERIALMODEL_H

#include "Material.h"

// Forward declarations
class ElasticityTensor;
class MaterialModel;
class VolumetricModel;

template<>
InputParameters validParams<MaterialModel>();

/**
 * MaterialModel is the base class for all solid mechanics material models in Elk.
 */
class MaterialModel : public Material
{
public:
  MaterialModel( const std::string & name,
                 InputParameters parameters );
  virtual ~MaterialModel();

  void testMe();

protected:

  enum DecompMethod
  {
    RashidApprox = 0,
    Eigen        = 1
  };

  bool _initialized;

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

  VariableGradient & _grad_disp_x;
  VariableGradient & _grad_disp_y;
  VariableGradient & _grad_disp_z;
  VariableGradient & _grad_disp_x_old;
  VariableGradient & _grad_disp_y_old;
  VariableGradient & _grad_disp_z_old;

  const bool _has_temp;
  VariableValue & _temperature;
  VariableValue & _temperature_old;

  const std::vector<std::string> _volumetric_model_names;
  std::vector<VolumetricModel*> _volumetric_models;

  DecompMethod _decomp_method;
  const Real _alpha;

  MaterialProperty<RealTensorValue> & _stress;
  MaterialProperty<RealTensorValue> & _stress_old;

  MaterialProperty<ColumnMajorMatrix> & _Jacobian_mult;
  ColumnMajorMatrix _strain_increment;
  ColumnMajorMatrix _incremental_rotation;
  ColumnMajorMatrix _Uhat;


  std::vector<ColumnMajorMatrix> _Fhat;
  std::vector<ColumnMajorMatrix> _Fbar;


  virtual void subdomainSetup();


  virtual void computeProperties();



  /// Modify increment for things like thermal strain
  virtual void modifyStrain();

  /// Compute the stress (sigma += deltaSigma)
  virtual void computeStress() = 0;

  /// Rotate stress to current configuration
  virtual void finalizeStress();


  void computeIncrementalDeformationGradient( std::vector<ColumnMajorMatrix> & Fhat);
  void computeStrainIncrement( const ColumnMajorMatrix & Fhat);
  void computePolarDecomposition( const ColumnMajorMatrix & Fhat);
  virtual void computePreconditioning();
  int delta(int i, int j);

  void computeStrainAndRotationIncrement( const ColumnMajorMatrix & Fhat);


  void fillMatrix( const VariableGradient & grad_x,
                   const VariableGradient & grad_y,
                   const VariableGradient & grad_z,
                   ColumnMajorMatrix & A );

  Real detMatrix( const ColumnMajorMatrix & A );

  void invertMatrix( const ColumnMajorMatrix & A,
                     ColumnMajorMatrix & Ainv );

  RealTensorValue rotateSymmetricTensor( const ColumnMajorMatrix & R, const RealTensorValue & T );

  void
  elasticityTensor( ElasticityTensor * e );

  ElasticityTensor *
  elasticityTensor() const
  {
    return _elasticity_tensor;
  }


private:
  ElasticityTensor * _elasticity_tensor;

};



#endif
