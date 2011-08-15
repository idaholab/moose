#ifndef MATERIALMODEL_H
#define MATERIALMODEL_H

#include "SolidModel.h"

// Forward declarations
class MaterialModel;
class SymmElasticityTensor;
class VolumetricModel;

template<>
InputParameters validParams<MaterialModel>();

/**
 * MaterialModel is the base class for all solid mechanics material models in Elk.
 */
class MaterialModel : public SolidModel
{
public:
  MaterialModel( const std::string & name,
                 InputParameters parameters );
  virtual ~MaterialModel();

  void testMe();

  static Real detMatrix( const ColumnMajorMatrix & A );

  static void invertMatrix( const ColumnMajorMatrix & A,
                            ColumnMajorMatrix & Ainv );

  static void rotateSymmetricTensor( const ColumnMajorMatrix & R, const RealTensorValue & T,
                                     RealTensorValue & result );
  static void rotateSymmetricTensor( const ColumnMajorMatrix & R, const SymmTensor & T,
                                     SymmTensor & result );

protected:

  enum DecompMethod
  {
    RashidApprox = 0,
    Eigen        = 1
  };

  VariableGradient & _grad_disp_x;
  VariableGradient & _grad_disp_y;
  VariableGradient & _grad_disp_z;
  VariableGradient & _grad_disp_x_old;
  VariableGradient & _grad_disp_y_old;
  VariableGradient & _grad_disp_z_old;

  DecompMethod _decomp_method;

  ColumnMajorMatrix _incremental_rotation;
  ColumnMajorMatrix _Uhat;

  std::vector<ColumnMajorMatrix> _Fhat;
  std::vector<ColumnMajorMatrix> _Fbar;

  virtual void elementInit();

  virtual void computeStrain();

  /// Modify increment for things like thermal strain
  virtual void modifyStrain();

  /// Compute the stress (sigma += deltaSigma)
  virtual void computeStress() = 0;

  /// Rotate stress to current configuration
  virtual void finalizeStress();


  void computeIncrementalDeformationGradient( std::vector<ColumnMajorMatrix> & Fhat);
  void computeStrainIncrement( const ColumnMajorMatrix & Fhat);
  void computePolarDecomposition( const ColumnMajorMatrix & Fhat);
  int delta(int i, int j);

  void computeStrainAndRotationIncrement( const ColumnMajorMatrix & Fhat);


  void fillMatrix( const VariableGradient & grad_x,
                   const VariableGradient & grad_y,
                   const VariableGradient & grad_z,
                   ColumnMajorMatrix & A );

};



#endif
