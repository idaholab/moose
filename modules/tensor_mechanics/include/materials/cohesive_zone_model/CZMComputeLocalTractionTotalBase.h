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
 * Base class used to implement traction separetion laws for materials whose beahvior can be
 * described using only the total displacement jump. Traction separetion laws shall compute the
 * interface traction using the interface coordinate system, and traction derivtives w.r.t. to the
 * interface displacement jump. Interface traction and related derivatives should be implemented
 * overriding the computeInterfaceTractionAndDerivatives method. The interface coordinate system
 * assumes the three component of the traction and disaplcement jump being ordered as [N,S1,S2],
 * where N is the normal component and S1, S2 two orthogonal tangential components. The model also
 * assumes isotropic behavior in the tangential directions.
 */
class CZMComputeLocalTractionTotalBase : public CZMComputeLocalTractionBase
{
public:
  static InputParameters validParams();
  CZMComputeLocalTractionTotalBase(const InputParameters & parameters);
};
