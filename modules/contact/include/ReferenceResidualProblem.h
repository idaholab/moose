/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef REFERENCERESIDUALPROBLEM_H
#define REFERENCERESIDUALPROBLEM_H

#include "FEProblem.h"

class ReferenceResidualProblem;

template<>
InputParameters validParams<ReferenceResidualProblem>();

/**
 * FEProblem derived class to enable convergence checking relative to a user-specified postprocessor
 */
class ReferenceResidualProblem : public FEProblem
{
public:
  ReferenceResidualProblem(const std::string & name, InputParameters params);
  virtual ~ReferenceResidualProblem();

  virtual void initialSetup();
  virtual void timestepSetup();
  void updateReferenceResidual();
  virtual MooseNonlinearConvergenceReason checkNonlinearConvergence(std::string &msg,
                                                                    const PetscInt it,
                                                                    const Real xnorm,
                                                                    const Real snorm,
                                                                    const Real fnorm,
                                                                    const Real rtol,
                                                                    const Real stol,
                                                                    const Real abstol,
                                                                    const PetscInt nfuncs,
                                                                    const PetscInt max_funcs,
                                                                    const Real ref_resid,
                                                                    const Real div_threshold);

  bool checkConvergenceIndividVars(const Real fnorm,
                                   const Real abstol,
                                   const Real rtol,
                                   const Real ref_resid);

protected:
  std::vector<std::string> _solnVarNames;
  std::vector<std::string> _refResidVarNames;
  std::vector<unsigned int> _solnVars;
  std::vector<unsigned int> _refResidVars;
  Real _accept_mult;
  int _accept_iters;
  std::vector<Real> _refResid;
  std::vector<Real> _resid;
};

#endif /* REFERENCERESIDUALPROBLEM_H */
