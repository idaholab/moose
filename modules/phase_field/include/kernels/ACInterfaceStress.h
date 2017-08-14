/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ACINTERFACESTRESS_H
#define ACINTERFACESTRESS_H

#include "Kernel.h"
#include "RankThreeTensor.h"

class ACInterfaceStress;
class RankTwoTensor;

template <>
InputParameters validParams<ACInterfaceStress>();

/**
 * Compute the Allen-Cahn interface stress driving force contribution
 * \f$ -\frac12L\left(\nabla \frac{\partial \sigma_{int}}{\partial\nabla\eta_i}:\epsilon, \psi_m
 * \right) \f$
 */
class ACInterfaceStress : public Kernel
{
public:
  ACInterfaceStress(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Mobility
  const MaterialProperty<Real> & _L;

  ///@{ Strain base name and property
  std::string _base_name;
  const MaterialProperty<RankTwoTensor> & _strain;
  ///@}

  /// interface stress
  const Real _stress;

  /// d sigma/d(grad eta), derivative of interface stress tensor with order parameter gradient
  RankThreeTensor _dS;

  /// derivative of _dS w.r.t. the finite element coefficients for the Jacobian calculation
  RankThreeTensor _ddS;
};

#endif // ACINTERFACESTRESS_H
