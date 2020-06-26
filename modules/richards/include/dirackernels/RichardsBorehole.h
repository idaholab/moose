//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "PeacemanBorehole.h"
#include "RichardsVarNames.h"
#include "RichardsDensity.h"
#include "RichardsRelPerm.h"
#include "RichardsSeff.h"

/**
 * Approximates a borehole by a sequence of Dirac Points
 */
class RichardsBorehole : public PeacemanBorehole
{
public:
  /**
   * Creates a new RichardsBorehole
   * This sets all the variables, but also
   * reads the file containing the lines of the form
   * radius x y z
   * that defines the borehole geometry.
   * It also calculates segment-lengths and rotation matrices
   * needed for computing the borehole well constant
   */
  static InputParameters validParams();

  RichardsBorehole(const InputParameters & parameters);

  /**
   * Computes the residual.  This just
   * calls prepareNodalValues, if _fully_upwind
   * then calls DiracKernel::computeResidual
   */
  virtual void computeResidual();

  /**
   * Computes the Qp residual
   */
  virtual Real computeQpResidual();

  /**
   * Computes the Jacobian.  This just
   * calls prepareNodalValues, if _fully_upwind
   * then calls DiracKernel::computeJacobian
   */
  virtual void computeJacobian();

  /**
   * Computes the diagonal part of the jacobian
   */
  virtual Real computeQpJacobian();

  /**
   * Computes the off-diagonal part of the jacobian
   * Note: at March2014 this is never called since
   * moose does not have this functionality.  Hence
   * as of March2014 this has never been tested.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

protected:
  /// Whether to use full upwinding
  const bool _fully_upwind;

  /// Defines the richards variables in the simulation
  const RichardsVarNames & _richards_name_UO;

  /// number of richards variables
  const unsigned int _num_p;

  /// The moose internal variable number of the richards variable of this Dirac Kernel
  const unsigned int _pvar;

  /// user object defining the density.  Only used if _fully_upwind = true
  const RichardsDensity * const _density_UO;

  /// user object defining the effective saturation.  Only used if _fully_upwind = true
  const RichardsSeff * const _seff_UO;

  /// user object defining the relative permeability.  Only used if _fully_upwind = true
  const RichardsRelPerm * const _relperm_UO;

  /// number of nodes in this element.  Only used if _fully_upwind = true
  unsigned int _num_nodes;

  /**
   * nodal values of mobility = density*relperm/viscosity
   * These are used if _fully_upwind = true
   */
  std::vector<Real> _mobility;

  /**
   * d(_mobility)/d(variable_ph)  (variable_ph is the variable for phase=ph)
   * These are used in the jacobian calculations if _fully_upwind = true
   */
  std::vector<std::vector<Real>> _dmobility_dv;

  /// fluid porepressure (or porepressures in case of multiphase)
  const MaterialProperty<std::vector<Real>> & _pp;

  /// d(porepressure_i)/d(variable_j)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dpp_dv;

  /// fluid viscosity
  const MaterialProperty<std::vector<Real>> & _viscosity;

  /// material permeability
  const MaterialProperty<RealTensorValue> & _permeability;

  /// deriviatves of Seff wrt variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _dseff_dv;

  /// relative permeability
  const MaterialProperty<std::vector<Real>> & _rel_perm;

  /// d(relperm_i)/d(variable_j)
  const MaterialProperty<std::vector<std::vector<Real>>> & _drel_perm_dv;

  /// fluid density
  const MaterialProperty<std::vector<Real>> & _density;

  /// d(density_i)/d(variable_j)
  const MaterialProperty<std::vector<std::vector<Real>>> & _ddensity_dv;

  /**
   * Holds the values of pressures at all the nodes of the element
   * Only used if _fully_upwind = true
   * Eg:
   * _ps_at_nodes[_pvar] is a pointer to this variable's nodal porepressure values
   * So: (*_ps_at_nodes[_pvar])[i] = _var.dofValues()[i] = porepressure of pressure-variable _pvar
   * at node i
   */
  std::vector<const VariableValue *> _ps_at_nodes;

  /// calculates the nodal values of pressure, mobility, and derivatives thereof
  void prepareNodalValues();

  /**
   * Calculates Jacobian
   * @param wrt_num differentiate the residual wrt this Richards variable
   */
  Real jac(unsigned int wrt_num);
};
