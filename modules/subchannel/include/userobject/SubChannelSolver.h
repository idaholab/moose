#pragma once

#include "GeneralUserObject.h"
#include "Coupleable.h"
#include "SubChannelMesh.h"

class SubChannelSolver : public GeneralUserObject, public Coupleable
{
public:
  explicit SubChannelSolver(const InputParameters & params);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  SubChannelMesh * _mesh;
  MooseVariableFEBase & _mdot_var;
  MooseVariableFEBase & _SumWij_var;
  MooseVariableFEBase & _SumWijh_var;
  MooseVariableFEBase & _SumWijPrimeDhij_var;
  MooseVariableFEBase & _SumWijPrimeDUij_var;
  MooseVariableFEBase & _P_var;
  MooseVariableFEBase & _h_var;
  MooseVariableFEBase & _T_var;
  MooseVariableFEBase & _rho_var;
  MooseVariableFEBase & _S_flow_var;
  MooseVariableFEBase & _S_crossflow_var;
  MooseVariableFEBase & _w_perim_var;
  MooseVariableFEBase & _q_prime_var;
  Real _mflux_in;
  Real _T_in;
  Real _P_out;

public:
  static InputParameters validParams();
};
