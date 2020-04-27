//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "RankTwoTensorForward.h"
#include "RankFourTensorForward.h"

/**
 * Compute the Allen-Cahn interface stress driving force contribution
 * \f$ -\frac12L\left(\nabla \frac{\partial \sigma_{int}}{\partial\nabla\eta_i}:\epsilon, \psi_m
 * \right) \f$
 */
class ACInterfaceStress : public Kernel
{
public:
  static InputParameters validParams();

  ACInterfaceStress(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Mobility
  const MaterialProperty<Real> & _L;

  ///@{ Strain base name and property
  const std::string _base_name;
  const MaterialProperty<RankTwoTensor> & _strain;
  ///@}

  /// interface stress
  const Real _stress;

  /// d sigma/d(grad eta), derivative of interface stress tensor with order parameter gradient
  RankThreeTensor _dS;

  /// derivative of _dS w.r.t. the finite element coefficients for the Jacobian calculation
  RankThreeTensor _ddS;
};
