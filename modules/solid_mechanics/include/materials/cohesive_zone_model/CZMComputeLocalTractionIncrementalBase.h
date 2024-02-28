//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CZMComputeLocalTractionBase.h"

/**
 *Base class used to implement traction separetion laws requiring an incremental formulation.
 *For incremental cohesive constituve models, the user is responsble for calulating the interface
 *traction increment and its derivatives w.r.t. the interface displacement jump. The interface
 *traction increment and related derivatives should be implemented overriding the
 *computeInterfaceTractionIncrementAndDerivatives method. The interface coordinate system assumes
 *the three component of the traction and disaplcement jump being ordered as [N,S1,S2], where N is
 *the normal component and S1, S2 two orthogonal tangential components. The model also assumes
 *isotropic behavior in the tangential directions.
 */

class CZMComputeLocalTractionIncrementalBase : public CZMComputeLocalTractionBase
{
public:
  static InputParameters validParams();
  CZMComputeLocalTractionIncrementalBase(const InputParameters & parameters);

protected:
  /// method used to compute the total traction
  void computeInterfaceTractionAndDerivatives() override;

  /// method used to compute the traction increment and its derivatives
  virtual void computeInterfaceTractionIncrementAndDerivatives() = 0;

  /// the value of the interface traction increment
  MaterialProperty<RealVectorValue> & _interface_traction_inc;

  /// the old interface traction value
  const MaterialProperty<RealVectorValue> & _interface_traction_old;

  /// The displacment jump  incremenet in local coordinates
  MaterialProperty<RealVectorValue> & _interface_displacement_jump_inc;

  /// The old interface displacment jump
  const MaterialProperty<RealVectorValue> & _interface_displacement_jump_old;
};
