/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ACSEDGPOLY_H
#define ACSEDGPOLY_H

#include "ACBulk.h"

/**
 * This kernel adds the contribution of stored energy associated with dislocations to grain growth
 * This allows us to simulate recrystallization.This kernel depends grain_index instead of op_index
 * to take full advantage of Grain Tracker
 * So a grain_tracker UserObject must be used. If you want use OPs equal to grain_num, you can use
 * the fake grain_tracker UserObject,e.g., FauxGrainTracker
 */

// Forward Declarations
class ACSEDGPoly;
class GrainTrackerInterface;

template <>
InputParameters validParams<ACSEDGPoly>();

class ACSEDGPoly : public ACBulk<Real>
{
public:
  ACSEDGPoly(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);

  const unsigned int _op_num;

  std::vector<const VariableValue *> _vals;
  std::vector<unsigned int> _vals_var;

  /// the prefactor needed to calculate the deformation energy from dislocation density
  const MaterialProperty<Real> & _beta;

  /// the average/effective dislocation density
  const MaterialProperty<Real> & _rho_eff;

  /// dislocation density in grain i
  const MaterialProperty<Real> & _Disloc_Den_i;

  /// number of deformed grains
  unsigned int _deformed_grain_num;

  /// Grain tracker object
  const GrainTrackerInterface & _grain_tracker;

  /// index of the OP the kernel is currently acting on
  unsigned int _op_index;
};

#endif // ACSEDGPOLY_H
