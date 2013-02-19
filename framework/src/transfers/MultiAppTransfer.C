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

#include "MultiAppTransfer.h"

#include "Transfer.h"
#include "MooseTypes.h"


template<>
InputParameters validParams<MultiAppTransfer>()
{
  InputParameters params = validParams<Transfer>();
  params.addRequiredParam<MultiAppName>("multi_app", "The name of the MultiApp to use.");

  params.addRequiredParam<MooseEnum>("direction", MultiAppTransfer::directions(), "Whether this Transfer will be 'to' or 'from' a MultiApp.");

  return params;
}

MultiAppTransfer::MultiAppTransfer(const std::string & name, InputParameters parameters) :
    Transfer(name, parameters),
    _multi_app(_fe_problem.getMultiApp(getParam<MultiAppName>("multi_app"))),
    _direction(getParam<MooseEnum>("direction"))
{
}
