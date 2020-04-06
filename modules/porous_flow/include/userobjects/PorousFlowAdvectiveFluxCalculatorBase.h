//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AdvectiveFluxCalculatorBase.h"
#include "PorousFlowDictator.h"

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
  static InputParameters validParams();

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
   * Returns, r, where r[global node k][a] = d(K[node_i][node_j])/d(porous_flow_variable[global node
   * k][porous_flow_variable a])
   * @param node_i global node id param node_j global node id
   */
  const std::map<dof_id_type, std::vector<Real>> & getdK_dvar(dof_id_type node_i,
                                                              dof_id_type node_j) const;

  virtual void buildCommLists() override;
  virtual void exchangeGhostedInfo() override;

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

  /// _du_dvar[sequential_i][a] = d(u[global version of sequential node i])/d(porous_flow_variable[a])
  std::vector<std::vector<Real>> _du_dvar;

  /// Whether _du_dvar has been computed by the local thread
  std::vector<bool> _du_dvar_computed_by_thread;

  /**
   * _dkij_dvar[sequential_i][j][global_k][a] =
   * d(K[sequential_i][j])/d(porous_flow_variable[global_k][porous_flow_variable a]) Here j is the
   * j^th connection to sequential node sequential_i, and node k must be connected to node i
   */
  std::vector<std::vector<std::map<dof_id_type, std::vector<Real>>>> _dkij_dvar;

  /// _dflux_out_dvars[sequential_i][global_j][pvar] = d(flux_out[global version of sequential_i])/d(porous_flow_variable pvar at global node j)
  std::vector<std::map<dof_id_type, std::vector<Real>>> _dflux_out_dvars;

  /**
   * _triples_to_receive[proc_id] indicates the dk(i, j)/du_nodal information that we will receive
   * from proc_id.  _triples_to_receive is first built (in buildCommLists()) using global node IDs,
   * but after construction, a translation to sequential node IDs and the index of connections is
   * performed, for efficiency.
   * The result is that, for i a multiple of 3, we will receive
   * _dkij_dvar[_triples_to_receive[proc_id][i]][_triples_to_receive[proc_id][i +
   * 1]][_triples_to_receive[proc_id][i + 2]][:] from processor proc_id
   */
  std::map<processor_id_type, std::vector<dof_id_type>> _triples_to_receive;

  /**
   * _triples_to_send[proc_id] indicates the dk(i, j)/du_nodal information that we will send to
   * proc_id.  _triples_to_send is first built (in buildCommLists()) using global node IDs, but
   * after construction, a translation to sequential node IDs and the index of connections is
   * performed, for efficiency.
   * The result is that, for i a multiple of 3, we will send
   * _dkij_dvar[_triples_to_send[proc_id][i]][_triples_to_send[proc_id][i +
   * 1]][_triples_to_send[proc_id][i + 2]][:] to processor proc_id
   */
  std::map<processor_id_type, std::vector<dof_id_type>> _triples_to_send;

  /// Flag to check whether permeabiity derivatives are non-zero
  const bool _perm_derivs;
};
