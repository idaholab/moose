//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVVariable.h"

class INSFVNoSlipWallBC;
class InputParameters;

class INSFVVelocityVariable : public INSFVVariable
{
public:
  INSFVVelocityVariable(const InputParameters & params);

  static InputParameters validParams();

  using INSFVVariable::adGradSln;
  const VectorValue<ADReal> & adGradSln(const Elem * const elem) const override;

protected:
  bool isDirichletBoundaryFace(const FaceInfo & fi) const override;

  const ADReal & getDirichletBoundaryFaceValue(const FaceInfo & fi) const override;

  /**
   * @return a pair with the first item indicating whether there is a no slip wall bc for the
   * passed-in \p fi and the second item pointing to the corresponding bc object if true
   */
  std::pair<bool, const INSFVNoSlipWallBC *> getNoSlipWallBC(const FaceInfo & fi) const;
};
