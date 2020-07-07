#pragma once

#include "ExternalProblem.h"
#include "SubChannelApp.h"
#include "SubChannelMesh.h"

class SubChannel1PhaseProblem;

/**
 * This class implements the 1-phase steady state sub channel solver.
 */

class SubChannel1PhaseProblem : public ExternalProblem
{
public:
  SubChannel1PhaseProblem(const InputParameters & params);
  static InputParameters validParams();
  virtual void externalSolve() override;
  virtual void syncSolutions(Direction direction) override;
  virtual bool converged() override;

protected:
  SubChannelMesh & _subchannel_mesh;
  Real _mflux_in;
  Real _T_in;
  Real _P_out;
};
