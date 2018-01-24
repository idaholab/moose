/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NONLINEAR_H
#define NONLINEAR_H

#include "Element.h"

// Forward declarations
class MaterialModel;
class VolumetricModel;

namespace SolidMechanics
{

/**
 * Nonlinear is the base class for all large strain/rotation models.
 */
class Nonlinear : public Element
{
public:
  Nonlinear(SolidModel & solid_model, const std::string & name, const InputParameters & parameters);

  virtual ~Nonlinear();

  const ColumnMajorMatrix & incrementalRotation() const { return _incremental_rotation; }

  const std::vector<ColumnMajorMatrix> & Fhat() const { return _Fhat; }

protected:
  enum DecompMethod
  {
    RashidApprox = 0,
    Eigen = 1
  };

  DecompMethod _decomp_method;

  ColumnMajorMatrix _incremental_rotation;
  ColumnMajorMatrix _Uhat;

  std::vector<ColumnMajorMatrix> _Fhat;
  std::vector<ColumnMajorMatrix> _Fbar;
  ColumnMajorMatrix _F;

  virtual void init();

  virtual void computeStrain(const unsigned qp,
                             const SymmTensor & total_strain_old,
                             SymmTensor & total_strain_new,
                             SymmTensor & strain_increment);

  virtual Real volumeRatioOld(unsigned /*qp*/) const { mooseError("volumeRatioOld not defined"); }

  /// Rotate stress to current configuration
  virtual void finalizeStress(std::vector<SymmTensor *> & t);

  virtual void computeIncrementalDeformationGradient(std::vector<ColumnMajorMatrix> & Fhat) = 0;
  void computeStrainIncrement(const ColumnMajorMatrix & Fhat, SymmTensor & strain_increment);
  void computePolarDecomposition(const ColumnMajorMatrix & Fhat);

  void computeStrainAndRotationIncrement(const ColumnMajorMatrix & Fhat,
                                         SymmTensor & strain_increment);
};

} // namespace solid_mechanics

#endif
