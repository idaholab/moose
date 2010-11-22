#ifndef MATERIALMODEL_H
#define MATERIALMODEL_H

#include "Material.h"

// Forward declarations
class MaterialModel;

template<>
InputParameters validParams<MaterialModel>();

/**
 * MaterialModel is the base class for all solid mechanics material models in Elk.
 */
class MaterialModel : public Material
{
public:
  MaterialModel( const std::string & name,
                 MooseSystem & moose_system,
                 InputParameters parameters );
  virtual ~MaterialModel() {}

  void testMe();

protected:
  VariableGradient & _grad_disp_x;
  VariableGradient & _grad_disp_y;
  VariableGradient & _grad_disp_z;
  VariableGradient & _grad_disp_x_old;
  VariableGradient & _grad_disp_y_old;
  VariableGradient & _grad_disp_z_old;

  const bool _has_temp;
  VariableValue & _temperature;
  VariableValue & _temperature_old;
  const Real _alpha;

  MaterialProperty<RealTensorValue> & _stress;
  MaterialProperty<RealTensorValue> & _stress_old;

  MaterialProperty<ColumnMajorMatrix> & _Jacobian_mult;



  /**
   * The current quadrature point.
   */
  unsigned int _qp;



  void computeProperties();

  /// Modify increment for things like thermal strain
  virtual void modifyStrain( ColumnMajorMatrix & strain_increment );

  /// Compute the stress (sigma += deltaSigma)
  virtual void computeStress( const ColumnMajorMatrix & strain_increment );

  /// Rotate stress to current configuration
  virtual void finalizeStress( const ColumnMajorMatrix & rot );


  void computeIncrementalDeformationGradient( std::vector<ColumnMajorMatrix> & Fhat );
  void computeStrainIncrement( const ColumnMajorMatrix & Fhat,
                               ColumnMajorMatrix & d );
  void computePolarDecomposition( const ColumnMajorMatrix & Fhat,
                                  ColumnMajorMatrix & R );



  void fillMatrix( const VariableGradient & grad_x,
                   const VariableGradient & grad_y,
                   const VariableGradient & grad_z,
                   ColumnMajorMatrix & A );

  Real detMatrix( const ColumnMajorMatrix & A );

  void invertMatrix( const ColumnMajorMatrix & A,
                     ColumnMajorMatrix & Ainv );

  RealTensorValue rotateSymmetricTensor( const ColumnMajorMatrix & R, const RealTensorValue & T );

};



#endif
