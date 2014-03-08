#ifndef ELEMENT_H
#define ELEMENT_H

#include "Material.h"
#include "InputParameters.h"
#include "SymmTensor.h"

// Forward declarations
class SolidModel;

namespace Elk
{
namespace SolidMechanics
{

/**
 * Element is the base class for all solid mechanics element formulations in Elk.
 */
class Element :
    // This is ugly.  The Element is not a Material and should not
    //   derive from it.
    // The intent is to create a set of helper classes for SolidModel.
    //   These will calculate strains (primarily), thus compartmentalizing
    //   this need, which varies by element formulation.
    // However, an Element needs many pieces of data and many functions
    //   (_qrule, _JxW, coupledGradient, declareProperty, etc., etc., etc.)
    //   that it cannot get easily.  Thus, the only realistic choice is
    //   inheritance to get access to the member data and interfaces
    //   required.  Ugh.
    // If only these things were publicly accessible.
    //
    // Just try to ignore the "is-a" and think "needs-the-interface-of."
    //
    // WARNING
    // Do not use _qp in classes derived from Element.  Use the _qp from the
    // class using Element.
    //
    public Material
{
public:
  Element( const std::string & name,
           InputParameters parameters );
  virtual ~Element();

  static Real detMatrix( const ColumnMajorMatrix & A );

  static void invertMatrix( const ColumnMajorMatrix & A,
                            ColumnMajorMatrix & Ainv );

  static void rotateSymmetricTensor( const ColumnMajorMatrix & R, const RealTensorValue & T,
                                     RealTensorValue & result );

  static void rotateSymmetricTensor( const ColumnMajorMatrix & R, const SymmTensor & T,
                                     SymmTensor & result );
  static void unrotateSymmetricTensor( const ColumnMajorMatrix & R, const SymmTensor & T,
                                     SymmTensor & result );

  static void polarDecompositionEigen( const ColumnMajorMatrix & Fhat, ColumnMajorMatrix & Rhat, SymmTensor & strain_increment );

  virtual void init() {}

  virtual void computeDeformationGradient( unsigned int /*qp*/, ColumnMajorMatrix & /*F*/)
  {
    mooseError("computeDeformationGradient not defined for element type used");
  }

  virtual void computeStrain( const unsigned qp,
                              const SymmTensor & total_strain_old,
                              SymmTensor & total_strain_new,
                              SymmTensor & strain_increment ) = 0;

  virtual Real volumeRatioOld(unsigned /*qp*/) const { return 1; }

  /// Rotate stress to current configuration
  virtual void finalizeStress( std::vector<SymmTensor*> & /*t*/ ) {}

  virtual unsigned int getNumKnownCrackDirs() const
  {
    return 0;
  }

  void fillMatrix( unsigned int qp,
                   const VariableGradient & grad_x,
                   const VariableGradient & grad_y,
                   const VariableGradient & grad_z,
                   ColumnMajorMatrix & A );

private:
  using Material::_qp;
};

} // namespace solid_mechanics
} // namespace elk


#endif
