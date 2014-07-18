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
#include "AutoPositionsMultiApp.h"

template<>
InputParameters validParams<AutoPositionsMultiApp>()
{
  InputParameters params = validParams<TransientMultiApp>();

  params.suppressParameter<std::vector<Point> >("positions");
  params.suppressParameter<FileName>("positions_file");

  return params;
}


AutoPositionsMultiApp::AutoPositionsMultiApp(const std::string & name, InputParameters parameters):
    TransientMultiApp(name, parameters)
{
}

AutoPositionsMultiApp::~AutoPositionsMultiApp()
{
}

void
AutoPositionsMultiApp::fillPositions()
{
  _positions.push_back(Point(1,1,1));
}
