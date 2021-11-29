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

/**
 * Read values from a VectorPostprocessor that is producing vectors that are "number of processors"
 * in length.  Puts the value for each processor into an elemental auxiliary field.
 */
class VectorPostprocessorVisualizationAux : public AuxKernel
{
public:
  static InputParameters validParams();

  VectorPostprocessorVisualizationAux(const InputParameters & parameters);

protected:
  /**
   * Note: this used for error checking.  It's done very late because VPP's don't fill in their
   * vectors until they are computed
   */
  virtual void timestepSetup() override;

  /**
   * Get the value from the vector and assign it to the element
   */
  virtual Real computeValue() override;

  /// Whether or not we're using a broadcast (replicated) vector
  bool _use_broadcast;

  /// Holds the values we want to display
  const ScatterVectorPostprocessorValue & _vpp_scatter;

  /// Holds the values we want to display
  const VectorPostprocessorValue & _vpp_vector;

  /// Optimization
  processor_id_type _my_pid;
};
