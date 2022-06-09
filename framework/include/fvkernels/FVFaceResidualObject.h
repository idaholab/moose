//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

class FaceInfo;

/**
 * Interface class for a finite volume residual object whose residuals are based on faces
 */
class FVFaceResidualObject
{
public:
  /**
   * Compute the residual on the supplied face
   */
  virtual void computeResidual(const FaceInfo & fi) = 0;

  /**
   * Compute the jacobian on the supplied face
   */
  virtual void computeJacobian(const FaceInfo & fi) = 0;

  /**
   * Compute the residual and Jacobian on the supplied face
   */
  virtual void computeResidualAndJacobian(const FaceInfo & fi) = 0;
};
