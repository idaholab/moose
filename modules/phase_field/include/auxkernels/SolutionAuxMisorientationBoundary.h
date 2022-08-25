//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolutionAux.h"

/**
 * AuxKernel for reading a solution from file and visualizing the location of
 * grain boundaries for specific grain boundary type.
 */
class SolutionAuxMisorientationBoundary : public SolutionAux
{
public:
  static InputParameters validParams();

  SolutionAuxMisorientationBoundary(const InputParameters & parameters);

protected:
  /**
   * Computes a value for a node or element depending on the type of kernel,
   * it also uses the 'direct' flag to extract values based on the dof if the
   * flag is set to true.
   * @ return the visualization for the location of specific grain boundaries.
   */
  virtual Real computeValue() override;

  /// The grain boundary type to calculate bnds parameter
  Real _gb_type_order;

  const unsigned int _op_num;
  const std::vector<const VariableValue *> _vals;
};
