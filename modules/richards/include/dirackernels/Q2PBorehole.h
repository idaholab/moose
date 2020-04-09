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
#include "RichardsDensity.h"
#include "RichardsRelPerm.h"

/**
 * Approximates a borehole by a sequence of Dirac Points.
 * This is for use by a Q2P model.
 */
class Q2PBorehole : public PeacemanBorehole
{
public:
  /**
   * Creates a new Q2PBorehole
   * This sets all the variables, but also
   * reads the file containing the lines of the form
   * radius x y z
   * that defines the borehole geometry.
   * It also calculates segment-lengths and rotation matrices
   * needed for computing the borehole well constant
   */
  static InputParameters validParams();

  Q2PBorehole(const InputParameters & parameters);

  /**
   * Computes the residual.  This just
   * calls prepareNodalValues
   * then calls DiracKernel::computeResidual
   */
  virtual void computeResidual();

  /**
   * Computes the Qp residual
   */
  virtual Real computeQpResidual();

  /**
   * Computes the Jacobian.  This just
   * calls prepareNodalValues
   * then calls DiracKernel::computeJacobian
   */
  virtual void computeJacobian();

  /**
   * Computes the diagonal part of the jacobian
   */
  virtual Real computeQpJacobian();

  /**
   * Computes the off-diagonal part of the jacobian
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

protected:
  /// fluid density
  const RichardsDensity & _density;

  /// fluid relative permeability
  const RichardsRelPerm & _relperm;

  /// the other variable in the 2-phase system (this is saturation if Variable=porepressure, and viceversa)
  const VariableValue & _other_var_nodal;

  /// the variable number of the other variable
  const unsigned int _other_var_num;

  /// whether the Variable for this BC is porepressure or not
  const bool _var_is_pp;

  /// viscosity
  const Real _viscosity;

  /// permeability
  const MaterialProperty<RealTensorValue> & _permeability;

  /// number of nodes in this element.
  unsigned int _num_nodes;

  /// nodal porepressure
  std::vector<Real> _pp;

  /// nodal saturation
  std::vector<Real> _sat;

  /// nodal mobility
  std::vector<Real> _mobility;

  /// nodal d(mobility)/d(porepressure)
  std::vector<Real> _dmobility_dp;

  /// nodal d(mobility)/d(saturation)
  std::vector<Real> _dmobility_ds;

  /// calculates the nodal values of pressure, mobility, and derivatives thereof
  void prepareNodalValues();

  /**
   * Calculates Jacobian
   * @param jvar differentiate the residual wrt this variable
   */
  Real jac(unsigned int jvar);
};
