//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

class ADHeatFluxFromHeatStructureBaseUserObject;

/**
 * Base class for handling heat flux between flow channels and heat structures
 *
 * Since variables on flow channels and heat structures are subdomain restricted and
 * they do not share mesh elements, we cannot use the usual MOOSE computeOffDiagJacobian
 * method.  To enable this flow channel/heat structure coupling, the heat flux and its
 * Jacobians are first computed in ADHeatFluxFromHeatStructureBaseUserObject.  This, class
 * pulls the data from the user object and puts the residuals and Jacobians into the right
 * spots.  For this to properly work, the child class has to implement getOffDiagVariableNumbers
 * and computeQpOffDiagJacobianNeighbor methods.
 */
class ADHeatFluxBaseBC : public ADIntegratedBC
{
public:
  ADHeatFluxBaseBC(const InputParameters & parameters);

protected:
  /// User object that computes the heat flux
  const ADHeatFluxFromHeatStructureBaseUserObject & _q_uo;
  /// Perimeter of a single unit of heat structure
  const Real _P_hs_unit;
  /// Number of units of heat structure
  const unsigned int _n_unit;
  /// Is the heat structure coordinate system cylindrical?
  const bool _hs_coord_system_is_cylindrical;
  /// Coordinate transformation
  const Real _hs_coord;
  /// Factor by which to scale term on the flow channel side for the heat structure side
  const Real _hs_scale;

public:
  static InputParameters validParams();
};
