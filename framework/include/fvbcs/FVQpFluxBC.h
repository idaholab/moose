//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"

/**
 * Base class for FV flux BCs that need qp-indexed solution values on both sides of a face.
 */
class FVQpFluxBC : public FVFluxBC
{
public:
  static InputParameters validParams();
  FVQpFluxBC(const InputParameters & params);

protected:
  /**
   * @return the value of u at the cell centroid on the subdomain on which u is defined. E.g. u is
   * defined on either the FaceInfo elem or FaceInfo neighbor subdomain
   */
  const ADReal & uOnUSub() const;

  /**
   * @return the value of u at the ghost cell centroid
   */
  const ADReal & uOnGhost() const;

  const ADVariableValue & _u;
  const ADVariableValue & _u_neighbor;
};
