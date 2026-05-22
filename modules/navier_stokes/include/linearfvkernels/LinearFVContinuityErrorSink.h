#pragma once

#include "LinearFVElementalKernel.h"

class LinearFVContinuityErrorSink : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();

  LinearFVContinuityErrorSink(const InputParameters & params);

protected:
  Real computeMatrixContribution() override;
  Real computeRightHandSideContribution() override;

  const Moose::Functor<Real> & _coefficient;
};
