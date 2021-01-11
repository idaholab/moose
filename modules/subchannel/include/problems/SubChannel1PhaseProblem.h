#pragma once

#include "ExternalProblem.h"
#include "SubChannelApp.h"
#include "SubChannelMeshBase.h"
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
  void computeSumWij(int iz);
  void computeMdot(int iz);
  void computeEnthalpy(int iz);
  void computeProperties(int iz);
  void computeDP(int iz);
  double computeFrictionFactor(double Re);

protected:
  Eigen::VectorXd Wij;
  Eigen::VectorXd Wij_old;
  Eigen::VectorXd WijPrime;
  Eigen::MatrixXd Wij_global;
  Eigen::MatrixXd Wij_global_old;
  Eigen::MatrixXd WijPrime_global;
  const Real _g_grav;
  SubChannelMeshBase & _subchannel_mesh;
  const SinglePhaseFluidProperties * _fp;
  SolutionHandle * mdot_soln;
  SolutionHandle * SumWij_soln;
  SolutionHandle * P_soln;
  SolutionHandle * DP_soln;
  SolutionHandle * h_soln;
  SolutionHandle * T_soln;
  SolutionHandle * rho_soln;
  SolutionHandle * S_flow_soln;
  SolutionHandle * w_perim_soln;
  SolutionHandle * q_prime_soln;
};
