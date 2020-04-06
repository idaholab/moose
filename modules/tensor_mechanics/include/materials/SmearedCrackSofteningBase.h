//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Conversion.h"
#include "InputParameters.h"
#include "Material.h"

// Forward declaration

/**
 * SmearedCrackSofteningBase is the base class for a set of models that define the
 * softening behavior of a crack under loading in a given direction.
 * These models are called by ComputeSmearedCrackingStress, so they
 * must have the compute=false flag set in the parameter list.
 */
class SmearedCrackSofteningBase : public Material
{
public:
  static InputParameters validParams();

  SmearedCrackSofteningBase(const InputParameters & parameters);

  /**
   * Compute the effect of the cracking release model on the stress
   * and stiffness in the direction of a single crack.
   * @param stress Stress in direction of crack
   * @param stiffness_ratio Ratio of damaged to original stiffness
   *                        in cracking direction
   * @param strain Strain in the current crack direction
   * @param crack_initiation_strain Strain in crack direction when crack
   *                                first initiated
   * @param crack_max_strain Maximum strain in crack direction
   * @param cracking_stress Threshold tensile stress for crack initiation
   * @param youngs_modulus Young's modulus
   */
  virtual void computeCrackingRelease(Real & stress,
                                      Real & stiffness_ratio,
                                      const Real strain,
                                      const Real crack_initiation_strain,
                                      const Real crack_max_strain,
                                      const Real cracking_stress,
                                      const Real youngs_modulus) = 0;

  ///@{ Retained as empty methods to avoid a warning from Material.C in framework. These methods are unused in all inheriting classes and should not be overwritten.
  void resetQpProperties() final {}
  void resetProperties() final {}
  ///@}
};
