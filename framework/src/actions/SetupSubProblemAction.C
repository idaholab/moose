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

#include "SetupSubProblemAction.h"
#include "Moose.h"
#include "FEProblem.h"
#include "Parser.h"
#include "Conversion.h"

template<>
InputParameters validParams<SetupSubProblemAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::string>("coord_type", "XYZ", "Type of the coordinate system");
  return params;
}

SetupSubProblemAction::SetupSubProblemAction(const std::string & name, InputParameters parameters) :
    Action(name, parameters),
    _coord_sys(getParam<std::string>("coord_type"))
{
}

SetupSubProblemAction::~SetupSubProblemAction()
{
}

void
SetupSubProblemAction::act()
{
  if (_parser_handle._problem != NULL)
  {
    SubProblem & subproblem = *_parser_handle._problem;
    subproblem.setCoordSystem(Moose::stringToEnum<Moose::CoordinateSystemType>(_coord_sys));
  }
}
