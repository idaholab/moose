#pragma once

#include "ExternalProblem.h"
#include "SubChannelApp.h"
#include "SubChannelMesh.h"
#include "SolutionHandle.h"
#include "SinglePhaseFluidProperties.h"

class SubChannel1PhaseProblem;

/**
 * This class implements the 1-phase steady state sub channel solver.
 */
class SubChannel1PhaseProblem : public ExternalProblem
{
public:
  SubChannel1PhaseProblem(const InputParameters & params);
  virtual ~SubChannel1PhaseProblem();
  static InputParameters validParams();
  virtual void externalSolve() override;
  virtual void syncSolutions(Direction direction) override;
  virtual bool converged() override;
  virtual void initialSetup() override;
  void computeWij(int iz);
  void computeSumWij(double SumSumWij, int iz);
  void computeMdot(int iz);
  void computeDP(int iz);

protected:
  Eigen::VectorXd Wij_old;
  Eigen::VectorXd WijPrime;
  Eigen::MatrixXd Wij_global_old;
  Eigen::VectorXd Wij;
  Eigen::MatrixXd Wij_global;
  const Real _g_grav;
  SubChannelMesh & _subchannel_mesh;
  const SinglePhaseFluidProperties * _fp;
  SolutionHandle * mdot_soln;
  SolutionHandle * SumWij_soln;
  SolutionHandle * SumWijh_soln;
  SolutionHandle * SumWijPrimeDhij_soln;
  SolutionHandle * SumWijPrimeDUij_soln;
  SolutionHandle * P_soln;
  SolutionHandle * DP_soln;
  SolutionHandle * h_soln;
  SolutionHandle * T_soln;
  SolutionHandle * rho_soln;
  SolutionHandle * S_flow_soln;
  SolutionHandle * w_perim_soln;
  SolutionHandle * q_prime_soln;
};
