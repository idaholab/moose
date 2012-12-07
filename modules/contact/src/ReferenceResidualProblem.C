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

#include "ReferenceResidualProblem.h"

#include "MooseApp.h"

template<>
InputParameters validParams<ReferenceResidualProblem>()
{
  InputParameters params = validParams<FEProblem>();
  params.addParam<std::vector<std::string> >("reference_residual","Set of postprocessors that calculate the reference residual to use in the relative convergence criterion");
  return params;
}

ReferenceResidualProblem::ReferenceResidualProblem(const std::string & name, InputParameters params) :
    FEProblem(name, params),
    _refResid(0.0),
    _haveReferenceResid(false)
{
  Moose::app->parser().extractParams("Problem", params);
  params.checkParams("Problem");

  if (params.isParamValid("reference_residual"))
    _refResidPPNames = params.get<std::vector<std::string> >("reference_residual");
  if (_refResidPPNames.size() > 0)
    _haveReferenceResid = true;
}

ReferenceResidualProblem::~ReferenceResidualProblem()
{}

void
ReferenceResidualProblem::timestepSetup()
{
  _refResid=0.0;
  FEProblem::timestepSetup();
}

void
ReferenceResidualProblem::updateReferenceResidual()
{
  NonlinearSystem & nonlinear_sys = getNonlinearSystem();
  if (_haveReferenceResid)
  {
    computeUserObjects(EXEC_CUSTOM);
    Real residsq=0.0;
    for (int i=0; i<_refResidPPNames.size(); ++i)
    {
      Real sqrtresid = getPostprocessorValue(_refResidPPNames[i]);
      residsq += sqrtresid*sqrtresid;
    }
    _refResid = std::sqrt(residsq);

    if (_refResid == 0.0)
    {
      _refResid = nonlinear_sys._initial_residual;
      std::cout<<"User-defined reference residual=0  Using default: "<<_refResid<<std::endl;
    }
    else
    {
      std::cout<<"User-defined reference residual: "<<_refResid<<std::endl;
    }
  }
  else
  {
    _refResid = nonlinear_sys._initial_residual;
  }
}

MooseNonlinearConvergenceReason
ReferenceResidualProblem::checkNonlinearConvergence(std::string &msg,
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
                                                    const Real /*ref_resid*/,
                                                    const Real div_threshold)
{
  updateReferenceResidual();

  MooseNonlinearConvergenceReason reason = FEProblem::checkNonlinearConvergence(msg,
                                                                                it,
                                                                                xnorm,
                                                                                snorm,
                                                                                fnorm,
                                                                                ttol,
                                                                                rtol,
                                                                                stol,
                                                                                abstol,
                                                                                nfuncs,
                                                                                max_funcs,
                                                                                _refResid,
                                                                                div_threshold);

//  std::cout<<msg<<std::endl; //Print convergence diagnostic message
  return(reason);
}
