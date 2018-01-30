//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENT_H
#define ELEMENT_H

#include "Material.h"
#include "InputParameters.h"
#include "SymmTensor.h"

// Forward declarations
class SolidModel;

namespace SolidMechanics
{

/**
 * Element is the base class for all of this module's solid mechanics element formulations.
 */
class Element : public Coupleable
{
public:
  Element(SolidModel & solid_model, const std::string & name, const InputParameters & parameters);
  virtual ~Element();

  static Real detMatrix(const ColumnMajorMatrix & A);

  static void invertMatrix(const ColumnMajorMatrix & A, ColumnMajorMatrix & Ainv);

  static void rotateSymmetricTensor(const ColumnMajorMatrix & R,
                                    const RealTensorValue & T,
                                    RealTensorValue & result);

  static void
  rotateSymmetricTensor(const ColumnMajorMatrix & R, const SymmTensor & T, SymmTensor & result);
  static void
  unrotateSymmetricTensor(const ColumnMajorMatrix & R, const SymmTensor & T, SymmTensor & result);

  static void polarDecompositionEigen(const ColumnMajorMatrix & Fhat,
                                      ColumnMajorMatrix & Rhat,
                                      SymmTensor & strain_increment);

  virtual void init() {}

  virtual void computeDeformationGradient(unsigned int /*qp*/, ColumnMajorMatrix & /*F*/)
  {
    mooseError("computeDeformationGradient not defined for element type used");
  }

  virtual void computeStrain(const unsigned qp,
                             const SymmTensor & total_strain_old,
                             SymmTensor & total_strain_new,
                             SymmTensor & strain_increment) = 0;

  virtual Real volumeRatioOld(unsigned /*qp*/) const { return 1; }

  /// Rotate stress to current configuration
  virtual void finalizeStress(std::vector<SymmTensor *> & /*t*/) {}

  virtual unsigned int getNumKnownCrackDirs() const { return 0; }

  void fillMatrix(unsigned int qp,
                  const VariableGradient & grad_x,
                  const VariableGradient & grad_y,
                  const VariableGradient & grad_z,
                  ColumnMajorMatrix & A);

protected:
  SolidModel & _solid_model;

private:
};

} // namespace solid_mechanics

#endif
