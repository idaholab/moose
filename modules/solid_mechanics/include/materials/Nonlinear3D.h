#ifndef NONLINEAR3D_H
#define NONLINEAR3D_H

#include "Element.h"

// Forward declarations
class MaterialModel;
class VolumetricModel;

namespace SolidMechanics
{

/**
 * Nonlinear3D is the base class for all 3D nonlinear solid mechanics material models.
 */
class Nonlinear3D : public Element
{
public:
  Nonlinear3D( SolidModel & solid_model,
               const std::string & name,
               InputParameters parameters );

  virtual ~Nonlinear3D();

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
  ColumnMajorMatrix _F;

  virtual void init();

  virtual void computeDeformationGradient( unsigned int qp, ColumnMajorMatrix & F);

  virtual void computeStrain( const unsigned qp,
                              const SymmTensor & total_strain_old,
                              SymmTensor & total_strain_new,
                              SymmTensor & strain_increment );

  virtual Real volumeRatioOld(unsigned qp) const;

  /// Rotate stress to current configuration
  virtual void finalizeStress( std::vector<SymmTensor*> & t );


  void computeIncrementalDeformationGradient( std::vector<ColumnMajorMatrix> & Fhat);
  void computeStrainIncrement( const ColumnMajorMatrix & Fhat,
                               SymmTensor & strain_increment );
  void computePolarDecomposition( const ColumnMajorMatrix & Fhat);

  void computeStrainAndRotationIncrement( const ColumnMajorMatrix & Fhat,
                                          SymmTensor & strain_increment );



};

} // namespace solid_mechanics


#endif
