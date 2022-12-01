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
#include "FunctorInterface.h"

class InputParameters;

class PINSFVPressureVariable : public INSFVPressureVariable, public FunctorInterface
{
public:
  PINSFVPressureVariable(const InputParameters & params);

  static InputParameters validParams();

  std::pair<bool, const Elem *> isExtrapolatedBoundaryFace(const FaceInfo & fi) const override;

  using INSFVPressureVariable::adGradSln;
  const VectorValue<ADReal> & adGradSln(const Elem * const elem,
                                        const bool correct_skewness = false) const override;

  void initialSetup() override;

protected:
  bool isDirichletBoundaryFace(const FaceInfo & fi) const override;

  ADReal getDirichletBoundaryFaceValue(const FaceInfo & fi) const override;

  std::tuple<bool, ADRealVectorValue, ADRealVectorValue> elemIsUpwind(const Elem & elem,
                                                                      const FaceInfo & fi) const;

  const Moose::Functor<ADReal> * _u;
  const Moose::Functor<ADReal> * _v;
  const Moose::Functor<ADReal> * _w;
  const Moose::Functor<ADReal> * _eps;
  const Moose::Functor<ADReal> * _rho;

private:
  mutable const Elem * _green_gauss_elem = nullptr;
};
