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
#include "RichardsDensity.h"
#include "RichardsRelPerm.h"
#include "Material.h"

// Forward Declarations

/**
 * This is a fully upwinded flux Kernel
 * The Variable of this Kernel should be the saturation
 *
 * The residual for the kernel is the darcy flux.
 * This is
 * R_i = int{mobility*flux_no_mob} = int{mobility*grad(pot)*permeability*grad(test_i)}
 * for node i.  where int is the integral over the element, and
 * pot = Porepressure - density*gravity.x
 *
 * However, in fully-upwind, the first step is to take the mobility outside the
 * integral.
 * R_i = mobility*int{flux_no_mob} = mobility*F_i
 * NOTE: R_i is exactly the mass flux flowing out of node i.
 * Similarly, F_i is a measure of fluid flowing out of node i.
 *
 * This leads to the definition of upwinding:
 *
 *   If F_i is positive then R_i = mobility_i * F_i
 *   That is, we use the upwind value of mobility.
 *
 * For the F_i<0 nodes we construct their R_i using mass conservation
 */
class Q2PSaturationFlux : public Kernel
{
public:
  static InputParameters validParams();

  Q2PSaturationFlux(const InputParameters & parameters);

protected:
  /**
   * Note that this is not the complete residual for the quadpoint
   * In computeResidual we sum over the quadpoints and then add
   * the upwind mobility parts
   */
  virtual Real computeQpResidual() override;

  /// This simply calls upwind
  virtual void computeResidual() override;

  /// this simply calls upwind
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  /// this simply calls upwind
  virtual void computeJacobian() override;

  /// the derivative of the flux without the upstream mobility terms
  Real computeQpJac(unsigned int dvar);

  /**
   * Do the upwinding for both the residual and jacobian
   * I've put both calculations in the same code to try to
   * reduce code duplication.  This is because when calculating
   * the jacobian we need to calculate the residual to see
   * which nodes are upwind and which are downwind
   */
  void upwind(bool compute_res, bool compute_jac, unsigned int jvar);

  /// calculates the nodal values of mobility, and derivatives thereof
  void prepareNodalValues();

  /// fluid density
  const RichardsDensity & _density;

  /// porepressure at the quadpoints
  const VariableValue & _pp;

  /// grad(porepressure) at the quadpoints
  const VariableGradient & _grad_pp;

  /// porepressure at the nodes
  const VariableValue & _pp_nodal;

  /// variable number of the porepressure variable
  unsigned int _pp_var;

  /// fluid relative permeability
  const RichardsRelPerm & _relperm;

  /// fluid viscosity
  Real _viscosity;

  /// gravity
  const MaterialProperty<RealVectorValue> & _gravity;

  /// permeability
  const MaterialProperty<RealTensorValue> & _permeability;

  /// number of nodes in the element
  unsigned int _num_nodes;

  /**
   * nodal values of mobility = density*relperm/viscosity
   * These are multiplied by _flux_no_mob to give the residual
   */
  std::vector<Real> _mobility;

  /**
   * d(_mobility)/d(porepressure)
   * These are used in the jacobian calculations
   */
  std::vector<Real> _dmobility_dp;

  /**
   * d(_mobility)/d(saturation)
   * These are used in the jacobian calculations
   */
  std::vector<Real> _dmobility_ds;
};
