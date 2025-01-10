//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class SolutionUserObjectBase;

/**
 * AuxKernel for reading a solution from file.
 * Creates a function that extracts values from a solution read from a file,
 * via a SolutionUserObject. It is possible to scale and add a constant to the
 * solution read.
 */
class SolutionAux : public AuxKernel
{
public:
  static InputParameters validParams();

  SolutionAux(const InputParameters & parameters);

  /**
   * Sets up the variable name for extraction from the SolutionUserObject
   */
  virtual void initialSetup() override;

protected:
  /**
   * Computes a value for a node or element depending on the type of kernel,
   * it also uses the 'direct' flag to extract values based on the dof if the
   * flag is set to true.
   * @ return The desired value of the solution for the current node or element
   */
  virtual Real computeValue() override;

  /// Reference to the SolutionUserObject storing the solution
  const SolutionUserObjectBase & _solution_object;

  /// The variable name of interest
  std::string _var_name;

  /// Flag for directly grabbing the data based on the dof
  bool _direct;

  /// Multiplier for the solution, the a of ax+b
  const Real _scale_factor;

  /// Additional factor added to the solution, the b of ax+b
  const Real _add_factor;
};
