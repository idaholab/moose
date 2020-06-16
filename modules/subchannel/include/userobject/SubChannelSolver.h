#pragma once

#include "GeneralUserObject.h"
#include "Coupleable.h"
#include "SubChannelMesh.h"

class SinglePhaseFluidProperties;

class SubChannelSolver : public GeneralUserObject, public Coupleable
{
public:
  explicit SubChannelSolver(const InputParameters & params);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  SubChannelMesh * _mesh;
  const MooseVariableFieldBase & _mdot_var;
  const MooseVariableFieldBase & _SumWij_var;
  const MooseVariableFieldBase & _SumWijh_var;
  const MooseVariableFieldBase & _SumWijPrimeDhij_var;
  const MooseVariableFieldBase & _SumWijPrimeDUij_var;
  const MooseVariableFieldBase & _P_var;
  const MooseVariableFieldBase & _h_var;
  const MooseVariableFieldBase & _T_var;
  const MooseVariableFieldBase & _rho_var;
  const MooseVariableFieldBase & _S_flow_var;
  const MooseVariableFieldBase & _S_crossflow_var;
  const MooseVariableFieldBase & _w_perim_var;
  const MooseVariableFieldBase & _q_prime_var;
  Real _mflux_in;
  Real _T_in;
  Real _P_out;
  /// Fluid properties user object
  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
