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

  virtual void init() {}

  virtual void computeStrain( const unsigned qp,
                              const SymmTensor & total_strain_old,
                              SymmTensor & total_strain_new,
                              SymmTensor & strain_increment ) = 0;

  /// Rotate stress to current configuration
  virtual void finalizeStress( SymmTensor & /*strain*/,
                               SymmTensor & /*stress*/ ) {}

  virtual unsigned int getNumKnownCrackDirs() const
  {
    return 0;
  }

  void fillMatrix( const VariableGradient & grad_x,
                   const VariableGradient & grad_y,
                   const VariableGradient & grad_z,
                   ColumnMajorMatrix & A );

};

} // namespace solid_mechanics
} // namespace elk


#endif
