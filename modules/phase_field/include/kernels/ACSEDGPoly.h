//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ACBulk.h"

/**
 * This kernel adds the contribution of stored energy associated with dislocations to grain growth
 * This allows us to simulate recrystallization. The formulation is based on:
 * S. Gentry and K. Thornton, IOP Conf. Series: Materials Science and
 * Engineering 89, 012024, (2015), and other works cited therein.
 * This kernel depends grain_index instead of op_index to take full advantage of Grain Tracker
 * So a grain_tracker UserObject must be used. If you want use OPs equal to grain_num, you can use
 * the fake grain_tracker UserObject,e.g., FauxGrainTracker
 */

// Forward Declarations
class GrainTrackerInterface;

class ACSEDGPoly : public ACBulk<Real>
{
public:
  static InputParameters validParams();

  ACSEDGPoly(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);

  const unsigned int _op_num;

  const std::vector<const VariableValue *> _vals;
  const std::vector<unsigned int> _vals_var;

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
