//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWADVECTIEFLUXCALCULATORBASE_H
#define POROUSFLOWADVECTIEFLUXCALCULATORBASE_H

#include "AdvectiveFluxCalculatorBase.h"
#include "PorousFlowDictator.h"

class PorousFlowAdvectiveFluxCalculatorBase;

template <>
InputParameters validParams<PorousFlowAdvectiveFluxCalculatorBase>();

/**
 * Base class to compute the advective flux of fluid in PorousFlow situations
 * using the Kuzmin-Turek FEM-TVD multidimensional stabilization scheme.
 *
 * The velocity is
 * U * (-permeability * (grad(P) - density * gravity))
 * with derived classes defining U.
 *
 * Much of this base class is spent with defining derivatives of the velocity
 * with respect to the PorousFlow variables.  As a first-time reader, if you
 * ignore these derivatives, you will find this class is quite simple.
 */
class PorousFlowAdvectiveFluxCalculatorBase : public AdvectiveFluxCalculatorBase
{
public:
  PorousFlowAdvectiveFluxCalculatorBase(const InputParameters & parameters);

  /**
   * Returns d(flux_out)/d(porous_flow_variables
   * @param[in] node_id global node id
   * @return deriv[j][pvar] = d(flux_out[node_id])/d(porous_flow_variable pvar at global node j)
   */
  const std::map<dof_id_type, std::vector<Real>> & getdFluxOut_dvars(unsigned node_id) const;

protected:
  virtual void timestepSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & uo) override;

  virtual Real computeVelocity(unsigned i, unsigned j, unsigned qp) const override;

  virtual void executeOnElement(dof_id_type global_i,
                                dof_id_type global_j,
                                unsigned local_i,
                                unsigned local_j,
                                unsigned qp) override;

  /**
   * Compute d(u)/d(porous_flow_variable)
   * @param i node number of the current element
   * @param pvar porous flow variable number
   */
  virtual Real computedU_dvar(unsigned i, unsigned pvar) const = 0;

  /**
   * Returns _du_dvar[node_i] which is the set of derivatives d(u)/d(porous_flow_variable)
   * This will have been computed during execute() by computedU_dvar()
   * @param node_i global node id
   */
  const std::vector<Real> & getdU_dvar(dof_id_type node_i) const;

  /**
   * Returns _du_dvar_computed_by_thread[node_i]
   * This is initialized to false in initialize() and potentially set to true during execute()
   * @param node_i global node id
   */
  bool getdU_dvarComputedByThread(dof_id_type node_i) const;

  /**
   * Returns, r, where r[global node k][a] = d(K[node_i][node_j])/d(porous_flow_variable[global node
   * k][porous_flow_variable a]) param node_i global node id param node_j global node id
   */
  const std::map<dof_id_type, std::vector<Real>> & getdK_dvar(dof_id_type node_i,
                                                              dof_id_type node_j) const;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Number of PorousFlow variables
  const unsigned _num_vars;

  /// Gravity
  const RealVectorValue _gravity;

  /// The phase
  const unsigned int _phase;

  /// Permeability of porous material
  const MaterialProperty<RealTensorValue> & _permeability;

  /// d(permeabiity)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealTensorValue>> & _dpermeability_dvar;

  /// d(permeabiity)/d(grad(PorousFlow variable))
  const MaterialProperty<std::vector<std::vector<RealTensorValue>>> & _dpermeability_dgradvar;

  /// Fluid density for each phase (at the qp)
  const MaterialProperty<std::vector<Real>> & _fluid_density_qp;

  /// Derivative of the fluid density for each phase wrt PorousFlow variables (at the qp)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_density_qp_dvar;

  /// Gradient of the pore pressure in each phase
  const MaterialProperty<std::vector<RealGradient>> & _grad_p;

  /// Derivative of Grad porepressure in each phase wrt grad(PorousFlow variables)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dgrad_p_dgrad_var;

  /// Derivative of Grad porepressure in each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<RealGradient>>> & _dgrad_p_dvar;

  /// FEType to use
  const FEType _fe_type;

  /// Kuzmin-Turek shape function
  const VariablePhiValue & _phi;

  /// grad(Kuzmin-Turek shape function)
  const VariablePhiGradient & _grad_phi;

  /// _du_dvar[i][a] = d(u[global node i])/d(porous_flow_variable[a])
  std::map<dof_id_type, std::vector<Real>> _du_dvar;

  /// Whether _du_dvar has been computed by the local thread
  std::map<dof_id_type, bool> _du_dvar_computed_by_thread;

  /// _dkij_dvar[i][j][k][a] = d(K[global node i][global node j])/d(porous_flow_variable[global_node k][porous_flow_variable a])
  std::map<dof_id_type, std::map<dof_id_type, std::map<dof_id_type, std::vector<Real>>>> _dkij_dvar;

  /// _dflux_out_dvars[i][j][pvar] = d(flux_out[global node i])/d(porous_flow_variable pvar at global node j)
  std::map<dof_id_type, std::map<dof_id_type, std::vector<Real>>> _dflux_out_dvars;
};

#endif // POROUSFLOWADVECTIEFLUXCALCULATORBASE_H
