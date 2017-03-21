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

#ifndef FEPROBLEM_H
#define FEPROBLEM_H

#include "FEProblemBase.h"
#include "NonlinearSystem.h"
#include "MooseEigenSystem.h"

class FEProblem;
class NonlinearSystem;

template <>
InputParameters validParams<FEProblem>();

/**
 * Specialization of SubProblem for solving nonlinear equations plus auxiliary equations
 *
 */
class FEProblem : public FEProblemBase
{
public:
  FEProblem(const InputParameters & parameters);

  virtual ~FEProblem();

  virtual bool getUseNonlinear() const { return _use_nonlinear; }
  virtual void setUseNonlinear(bool use_nonlinear) { _use_nonlinear = use_nonlinear; }

  virtual void setInputParametersFEProblem(InputParameters & parameters) override;

  NonlinearSystem & getNonlinearSystem() override { return *_nl_sys; }

protected:
  bool _use_nonlinear;
  NonlinearSystem * _nl_sys;
};

#endif /* FEPROBLEM_H */
