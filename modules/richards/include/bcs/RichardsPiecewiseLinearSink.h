//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"
#include "LinearInterpolation.h"
#include "Function.h"
#include "RichardsVarNames.h"
#include "RichardsDensity.h"
#include "RichardsRelPerm.h"
#include "RichardsSeff.h"

// Forward Declarations

/**
 * Applies a flux sink to a boundary
 * The sink is a piecewise linear function of
 * porepressure (the "variable") at the quad points.
 * This is specified by _sink_func.
 * In addition, this sink can be multiplied by:
 *  (1) the relative permeability of the fluid at the quad point.
 *  (2) perm_nn*density/viscosity, where perm_nn is the
 *      permeability tensor projected in the normal direction.
 *  (3) a Function (which can be time-dependent, for instance)
 * and divided by:
 *  (4) an area Postprocessor
 */
class RichardsPiecewiseLinearSink : public IntegratedBC
{
public:
  static InputParameters validParams();

  RichardsPiecewiseLinearSink(const InputParameters & parameters);

protected:
  virtual void computeResidual() override;

  virtual Real computeQpResidual() override;

  virtual void computeJacobian() override;

  virtual Real computeQpJacobian() override;

  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// whether to multiply the sink flux by permeability*density/viscosity
  bool _use_mobility;

  /// whether to multiply the sink flux by relative permeability
  bool _use_relperm;

  /// whether to use full upwinding
  bool _fully_upwind;

  /// piecewise-linear function of porepressure (this defines the strength of the sink)
  LinearInterpolation _sink_func;

  /// sink flux gets multiplied by this function
  const Function & _m_func;

  /// holds info about the names and values of richards variable in the simulation
  const RichardsVarNames & _richards_name_UO;

  /// number of richards variables
  unsigned int _num_p;

  /// the moose internal variable number corresponding to the porepressure of this sink flux
  unsigned int _pvar;

  /// user object defining the density.  Only used if _fully_upwind = true
  const RichardsDensity * const _density_UO;

  /// user object defining the effective saturation.  Only used if _fully_upwind = true
  const RichardsSeff * const _seff_UO;

  /// user object defining the relative permeability.  Only used if _fully_upwind = true
  const RichardsRelPerm * const _relperm_UO;

  /// area postprocessor.  if given then all bare_fluxes are divided by this quantity
  const PostprocessorValue & _area_pp;

  /// number of nodes in this element.  Only used if _fully_upwind = true
  unsigned int _num_nodes;

  /**
   * nodal values of fluid density
   * These are used if _fully_upwind = true
   */
  std::vector<Real> _nodal_density;

  /**
   * d(_nodal_density)/d(variable_ph)  (variable_ph is the variable for phase=ph)
   * These are used in the jacobian calculations if _fully_upwind = true
   */
  std::vector<std::vector<Real>> _dnodal_density_dv;

  /**
   * nodal values of relative permeability
   * These are used if _fully_upwind = true
   */
  std::vector<Real> _nodal_relperm;

  /**
   * d(_nodal_relperm)/d(variable_ph)  (variable_ph is the variable for phase=ph)
   * These are used in the jacobian calculations if _fully_upwind = true
   */
  std::vector<std::vector<Real>> _dnodal_relperm_dv;

  /// porepressure values (only the _pvar component is used)
  const MaterialProperty<std::vector<Real>> & _pp;

  /// d(porepressure_i)/d(variable_j)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dpp_dv;

  /// viscosity (only the _pvar component is used)
  const MaterialProperty<std::vector<Real>> & _viscosity;

  /// permeability
  const MaterialProperty<RealTensorValue> & _permeability;

  /**
   * derivative of effective saturation wrt variables
   * only _dseff_dv[_pvar][i] is used for i being all variables
   */
  const MaterialProperty<std::vector<std::vector<Real>>> & _dseff_dv;

  /// relative permeability (only the _pvar component is used)
  const MaterialProperty<std::vector<Real>> & _rel_perm;

  /// d(relperm_i)/d(variable_j)
  const MaterialProperty<std::vector<std::vector<Real>>> & _drel_perm_dv;

  /// fluid density (only the _pvar component is used)
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

  /// derivative of residual wrt the wrt_num Richards variable
  Real jac(unsigned int wrt_num);
};
