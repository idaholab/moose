#include "MooseTestApp.h"
#include "MooseTest.h"
#include "Moose.h"

template<>
InputParameters validParams<MooseTestApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}


MooseTestApp::MooseTestApp(const std::string & name, InputParameters parameters):
    MooseApp(name, parameters)
{
  srand(libMesh::processor_id());

  Moose::registerObjects(_factory);
  MooseTest::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  MooseTest::associateSyntax(_syntax, _action_factory);
}

MooseTestApp::~MooseTestApp()
{
}

