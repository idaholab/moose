#ifndef SUBCHANNELSOLVER_H
#define SUBCHANNELSOLVER_H

#include "GeneralUserObject.h"

#include "Coupleable.h"
#include "SubchannelMesh.h"

class SubchannelSolver;

template <>
InputParameters validParams<SubchannelSolver>();

class SubchannelSolver : public GeneralUserObject,
                         public Coupleable
{
public:
  explicit SubchannelSolver(const InputParameters & params);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  SubchannelMesh * _mesh;
  MooseVariableFEBase & _vz_var;
  MooseVariableFEBase & _P_var;
  MooseVariableFEBase & _h_var;
  MooseVariableFEBase & _T_var;
  MooseVariableFEBase & _rho_var;
  MooseVariableFEBase & _A_flow_var;
  MooseVariableFEBase & _w_perim_var;
  MooseVariableFEBase & _q_prime_var;
  Real _vz_in;
  Real _T_in;
  Real _P_in;
};
#endif // SUBCHANNELSOLVER_H
