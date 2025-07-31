//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HDGKernel.h"

/*
 * Imposes a singular perturbation on the component momentum equations penalizing discontinuities in
 * mass flux. Similar to \p MassFluxPenalty except it does not couple interior degrees of freedom on
 * neighboring elements, which makes this class useful in tandem with hybridized discretizations
 * because it supports static condensation
 */
class MassFluxPenaltyIPHDG : public HDGKernel
{
public:
  static InputParameters validParams();

  MassFluxPenaltyIPHDG(const InputParameters & parameters);

  virtual void computeResidual() override {}
  virtual void computeJacobian() override {}
  virtual void computeOffDiagJacobian(unsigned int) override {}
  virtual void computeResidualOnSide() override;
  virtual void computeJacobianOnSide() override;

protected:
  ADReal computeQpResidualOnSide();

  const MooseVariableField<Real> & _vel_x_var;
  const MooseVariableField<Real> & _vel_y_var;
  const MooseVariableField<Real> & _vel_x_face_var;
  const MooseVariableField<Real> & _vel_y_face_var;
  const ADVariableValue & _vel_x;
  const ADVariableValue & _vel_y;
  const ADVariableValue & _vel_x_face;
  const ADVariableValue & _vel_y_face;
  const MooseArray<std::vector<Real>> & _vel_x_phi;
  const MooseArray<std::vector<Real>> & _vel_y_phi;
  const MooseArray<std::vector<Real>> & _vel_x_face_phi;
  const MooseArray<std::vector<Real>> & _vel_y_face_phi;
  const unsigned short _comp;
  const Real _gamma;

  std::vector<Real> _residuals;
  std::vector<ADReal> _ad_residuals;

private:
  /**
   * Helper method to reduce code duplication, this will multiply quadrature point residuals
   * corresponding to the jump in mass flux by the passed-in \p test functions (corresponding to
   * either interior or face test functions) and \p sign (+1 or -1 respectively)
   */
  template <typename T>
  void computeOnSideHelper(std::vector<T> & residuals,
                           const MooseArray<std::vector<Real>> & test,
                           Real sign);
};
