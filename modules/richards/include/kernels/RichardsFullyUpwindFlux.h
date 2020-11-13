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
#include "RichardsVarNames.h"
#include "RichardsDensity.h"
#include "RichardsRelPerm.h"
#include "RichardsSeff.h"
#include "Material.h"

// Forward Declarations

/**
 * This is a fully upwinded version of RichardsFlux.
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
class RichardsFullyUpwindFlux : public Kernel
{
public:
  static InputParameters validParams();

  RichardsFullyUpwindFlux(const InputParameters & parameters);

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

  /**
   * holds info regarding the names of the Richards variables
   * and methods for extracting values of these variables
   */
  const RichardsVarNames & _richards_name_UO;

  /// number of richards variables
  unsigned int _num_p;

  /**
   * the index of this variable in the list of Richards variables
   * held by _richards_name_UO.  Eg
   * if richards_vars = 'pwater pgas poil' in the _richards_name_UO
   * and this kernel has variable = pgas, then _pvar = 1
   * This is used to index correctly into _viscosity, _seff, etc
   */
  unsigned int _pvar;

  /// user object defining the density
  const RichardsDensity & _density_UO;

  /// user object defining the effective saturation
  const RichardsSeff & _seff_UO;

  /// user object defining the relative permeability
  const RichardsRelPerm & _relperm_UO;

  /// viscosities
  const MaterialProperty<std::vector<Real>> & _viscosity;

  /// permeability*(grad(pressure) - density*gravity)  (a vector of these in the multiphase case)
  const MaterialProperty<std::vector<RealVectorValue>> & _flux_no_mob;

  /// d(_flux_no_mob)/d(variable)
  const MaterialProperty<std::vector<std::vector<RealVectorValue>>> & _dflux_no_mob_dv;

  /// d(_flux_no_mob)/d(grad(variable))
  const MaterialProperty<std::vector<std::vector<RealTensorValue>>> & _dflux_no_mob_dgradv;

  /// number of nodes in this element
  unsigned int _num_nodes;

  /**
   * nodal values of mobility = density*relperm/viscosity
   * These are multiplied by _flux_no_mob to give the residual
   */
  std::vector<Real> _mobility;

  /**
   * d(_mobility)/d(variable_ph)  (variable_ph is the variable for phase=ph)
   * These are used in the jacobian calculations
   */
  std::vector<std::vector<Real>> _dmobility_dv;

  /**
   * Holds the values of pressures at all the nodes of the element
   * Eg:
   * _ps_at_nodes[_pvar] is a pointer to this variable's nodal porepressure values
   * So: (*_ps_at_nodes[_pvar])[i] = _var.dofValues()[i] = value of porepressure at node i
   */
  std::vector<const VariableValue *> _ps_at_nodes;
};
