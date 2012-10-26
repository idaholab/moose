/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef FRICTIONALCONTACTPROBLEM_H
#define FRICTIONALCONTACTPROBLEM_H

#include "FEProblem.h"

class FrictionalContactProblem;

struct InteractionParams{
  Real _friction_coefficient;
  Real _slip_factor;
};

enum ContactState
{
  STICKING,
  SLIPPING,
  SLIPPED_TOO_FAR
};

template<>
InputParameters validParams<FrictionalContactProblem>();

/**
 * FEProblem derived class for frictional contact-specific callbacks
 */
class FrictionalContactProblem : public FEProblem
{
public:
  FrictionalContactProblem(const std::string & name, InputParameters params);
  virtual ~FrictionalContactProblem();
  virtual void timestepSetup();
  virtual bool shouldUpdateSolution();
  virtual bool updateSolution(NumericVector<Number>& vec_solution, NumericVector<Number>& ghosted_solution);
  virtual bool slipUpdate(NumericVector<Number>& vec_solution, const NumericVector<Number>& ghosted_solution);
  static ContactState calculateSlip(RealVectorValue &slip,
                                    Real &slip_residual,
                                    const RealVectorValue &normal,
                                    const RealVectorValue &residual,
                                    const RealVectorValue &incremental_slip,
                                    const RealVectorValue &stiffness,
                                    const Real friction_coefficient,
                                    const Real slip_factor,
                                    const int dim);
  virtual MooseNonlinearConvergenceReason checkNonlinearConvergence(std::string &msg, const int it, const Real xnorm, const Real snorm, const Real fnorm, Real &ttol, const Real rtol, const Real stol, const Real abstol, const int nfuncs, const int max_funcs);

protected:
  std::map<std::pair<int,int>,InteractionParams> _interaction_params;
  NonlinearVariableName _disp_x;
  NonlinearVariableName _disp_y;
  NonlinearVariableName _disp_z;
  AuxVariableName _residual_x;
  AuxVariableName _residual_y;
  AuxVariableName _residual_z;
  AuxVariableName _diag_stiff_x;
  AuxVariableName _diag_stiff_y;
  AuxVariableName _diag_stiff_z;
  AuxVariableName _inc_slip_x;
  AuxVariableName _inc_slip_y;
  AuxVariableName _inc_slip_z;

  Real _slip_residual;
  bool _do_slip_update;
  int _num_slip_iterations;
  int _min_slip_iters;
  int _max_slip_iters;
  int _slip_updates_per_iter;
  Real _target_contact_residual;
  Real _contact_slip_tol_factor;
};

#endif /* FRICTIONALCONTACTPROBLEM_H */
