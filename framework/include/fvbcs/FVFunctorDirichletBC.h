//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVDirichletBCBase.h"

template <bool is_ad>
class FVFunctorDirichletBCTempl : public FVDirichletBCBase
{
public:
  FVFunctorDirichletBCTempl(const InputParameters & parameters);

  static InputParameters validParams();

  ADReal boundaryValue(const FaceInfo & fi) const override;

private:
  /// The value for this BC
  const Moose::Functor<GenericReal<is_ad>> & _functor;
};

typedef FVFunctorDirichletBCTempl<false> FVFunctorDirichletBC;
typedef FVFunctorDirichletBCTempl<true> FVADFunctorDirichletBC;
