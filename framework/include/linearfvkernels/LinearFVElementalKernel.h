//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVKernel.h"
#include "ElemInfo.h"

/**
 * Finite volume kernel that contributes approximations of volumetric integral terms to the matrix
 * and right hand side of a linear system.
 */
class LinearFVElementalKernel : public LinearFVKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVElementalKernel(const InputParameters & params);

  virtual void addMatrixContribution() override;

  virtual void addRightHandSideContribution() override;

  /**
   * Set the current ElemInfo object.
   * @param elem_info The new ElemInfo which will be used as the current
   */
  virtual void setCurrentElemInfo(const ElemInfo * elem_info);

  /**
   * Set the coordinate system specific volume, the multiplication with
   * the transformation factor is done outside of the kernel.
   * @param volume the new coordinate specific volume
   */
  void setCurrentElemVolume(const Real volume) { _current_elem_volume = volume; }

  /// Computes the system matrix contribution for the given variable on the current element
  virtual Real computeMatrixContribution() = 0;

  /// Computes the right hand side contribution for the given variable on the current element
  virtual Real computeRightHandSideContribution() = 0;

protected:
  /// Pointer to the current element info
  const ElemInfo * _current_elem_info;

  /// The coordinate-specific element volume
  Real _current_elem_volume;

  /// The dof index for the current variable associated with the element
  dof_id_type _dof_id;
};
