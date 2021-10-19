//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADKernel.h"

template <bool is_ad>
class FunctorMatDiffusionTempl : public ADKernel
{
public:
  static InputParameters validParams();

  FunctorMatDiffusionTempl(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  const Moose::Functor<GenericReal<is_ad>> & _diff;
};

typedef FunctorMatDiffusionTempl<false> FunctorMatDiffusion;
typedef FunctorMatDiffusionTempl<true> ADFunctorMatDiffusion;
