//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralPostprocessor.h"
#include "BoundaryIntegrationFunctor.h"

template <bool is_ad>
class BoundaryFunctorIntegralTempl : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  BoundaryFunctorIntegralTempl(const InputParameters & parameters);

protected:
  /// Technically we should use finalize to compute the value
  Real getValue() const override;

  void initialize() override{};
  void execute() override{};
  void finalize() override{};

  std::unique_ptr<BoundaryIntegralFunctor<GenericReal<is_ad>>> _bif;
  std::unique_ptr<BoundaryAverageFunctor<GenericReal<is_ad>>> _baf;
  using FaceArg = Moose::FaceArg;
};

typedef BoundaryFunctorIntegralTempl<false> BoundaryFunctorIntegralOutput;
typedef BoundaryFunctorIntegralTempl<true> ADBoundaryFunctorIntegralOutput;
