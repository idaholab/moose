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

#include "ConstantDT.h"
#include "FEProblem.h"
#include "Transient.h"

namespace
{
const std::string dt_name = "dt";
const std::string growth_factor_name = "growth_factor";
}

template<>
InputParameters validParams<ConstantDT>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addRequiredParam<Real>( dt_name, "Size of the time step" );
  params.addRangeCheckedParam<Real>( growth_factor_name, 2, growth_factor_name + ">=1",
    "Maximum ratio of new to previous timestep sizes following a step that required the time"
    " step to be cut due to a failed solve." );
  return params;
}

ConstantDT::ConstantDT(const std::string & name, InputParameters parameters) :
    TimeStepper( name, parameters ),
    _constant_dt( getParam<Real>( dt_name ) ),
    _growth_factor( getParam<Real>( growth_factor_name ) )
{
}

Real
ConstantDT::computeInitialDT()
{
  return _constant_dt;
}

Real
ConstantDT::computeDT()
{
  return std::min( _constant_dt, _growth_factor * getCurrentDT() );
}
