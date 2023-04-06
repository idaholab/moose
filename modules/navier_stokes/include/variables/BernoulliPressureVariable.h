//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVPressureVariable.h"
#include "ADFunctorInterface.h"

class InputParameters;

/**
 * A special variable class for pressure which flags faces at which porosity jumps occur as
 * extrapolated or Dirichlet boundary faces. The downwind is flagged as an extrapolated boundary
 * face while the upwind face is flagged as a Dirichlet face. The upwind Dirichlet value is computed
 * using the downwind extrapolated pressure value and the Bernoulli equation
 */
class BernoulliPressureVariable : public INSFVPressureVariable, public ADFunctorInterface
{
public:
  BernoulliPressureVariable(const InputParameters & params);

  static InputParameters validParams();

  bool isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                  const Elem * elem,
                                  const Moose::StateArg & time) const override;

  void initialSetup() override;

protected:
  bool isDirichletBoundaryFace(const FaceInfo & fi,
                               const Elem * elem,
                               const Moose::StateArg & time) const override;

  ADReal getDirichletBoundaryFaceValue(const FaceInfo & fi,
                                       const Elem * elem,
                                       const Moose::StateArg & time) const override;

  /**
   * Checks to see whether the provided element is upwind of the provided face
   * @param elem the element to check whether it is upwind of the face
   * @param fi the face
   * @param time The time at which to evaluate the velocity
   * @return a pair in which the first member is whether the element is upwind of the face and the
   * second member is the evaluated face superficial velocity
   */
  std::pair<bool, ADRealVectorValue>
  elemIsUpwind(const Elem & elem, const FaceInfo & fi, const Moose::StateArg & time) const;

  /// The x-component of velocity
  const Moose::Functor<ADReal> * _u;
  /// The y-component of velocity
  const Moose::Functor<ADReal> * _v;
  /// The z-component of velocity
  const Moose::Functor<ADReal> * _w;
  /// The porosity
  const Moose::Functor<ADReal> * _eps;
  /// The density
  const Moose::Functor<ADReal> * _rho;
};
