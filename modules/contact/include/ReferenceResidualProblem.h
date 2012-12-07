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

  virtual void timestepSetup();
  void updateReferenceResidual();
  virtual MooseNonlinearConvergenceReason checkNonlinearConvergence(std::string &msg,
                                                                    const int it,
                                                                    const Real xnorm,
                                                                    const Real snorm,
                                                                    const Real fnorm,
                                                                    Real &ttol,
                                                                    const Real rtol,
                                                                    const Real stol,
                                                                    const Real abstol,
                                                                    const int nfuncs,
                                                                    const int max_funcs,
                                                                    const Real ref_resid,
                                                                    const Real div_threshold);

protected:
  std::vector<std::string> _refResidPPNames;
  Real _refResid;
  bool _haveReferenceResid;
};

#endif /* REFERENCERESIDUALPROBLEM_H */
