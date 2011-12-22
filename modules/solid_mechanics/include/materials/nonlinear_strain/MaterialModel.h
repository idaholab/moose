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

  static Real detMatrix( const ColumnMajorMatrix & A );

  static void invertMatrix( const ColumnMajorMatrix & A,
                            ColumnMajorMatrix & Ainv );

  static void rotateSymmetricTensor( const ColumnMajorMatrix & R, const RealTensorValue & T,
                                     RealTensorValue & result );
  static void rotateSymmetricTensor( const ColumnMajorMatrix & R, const SymmTensor & T,
                                     SymmTensor & result );

protected:


  /// Compute the stress (sigma += deltaSigma)
  virtual void computeStress() = 0;


  int delta(int i, int j);


  void fillMatrix( const VariableGradient & grad_x,
                   const VariableGradient & grad_y,
                   const VariableGradient & grad_z,
                   ColumnMajorMatrix & A );

};



#endif
