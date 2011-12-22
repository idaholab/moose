#ifndef NONLINEAR3D_H
#define NONLINEAR3D_H

#include "Element.h"

// Forward declarations
class MaterialModel;
class VolumetricModel;

namespace Elk
{
namespace SolidMechanics
{

/**
 * Nonlinear3D is the base class for all solid mechanics material models in Elk.
 */
class Nonlinear3D : public Element
{
public:
  Nonlinear3D( const std::string & name,
               InputParameters parameters );

  virtual ~Nonlinear3D();

  static Real detMatrix( const ColumnMajorMatrix & A );

  static void invertMatrix( const ColumnMajorMatrix & A,
                            ColumnMajorMatrix & Ainv );

  static void rotateSymmetricTensor( const ColumnMajorMatrix & R, const RealTensorValue & T,
                                     RealTensorValue & result );

  const ColumnMajorMatrix & incrementalRotation() const
  {
    return _incremental_rotation;
  }

  const std::vector<ColumnMajorMatrix> & Fhat() const
  {
    return _Fhat;
  }

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

  virtual void init();

  virtual void computeStrain( const unsigned qp,
                              const SymmTensor & total_strain_old,
                              SymmTensor & total_strain_new,
                              SymmTensor & strain_increment );

  /// Rotate stress to current configuration
  virtual void finalizeStress( SymmTensor & strain,
                               SymmTensor & stress );


  void computeIncrementalDeformationGradient( std::vector<ColumnMajorMatrix> & Fhat);
  void computeStrainIncrement( const ColumnMajorMatrix & Fhat,
                               SymmTensor & strain_increment );
  void computePolarDecomposition( const ColumnMajorMatrix & Fhat);
  int delta(int i, int j);

  void computeStrainAndRotationIncrement( const ColumnMajorMatrix & Fhat,
                                          SymmTensor & strain_increment );


  void fillMatrix( const VariableGradient & grad_x,
                   const VariableGradient & grad_y,
                   const VariableGradient & grad_z,
                   ColumnMajorMatrix & A );

};

} // namespace solid_mechanics
} // namespace elk


#endif
