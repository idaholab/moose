#include "MooseUnitApp.h"
#include "Moose.h"

template<>
InputParameters validParams<MooseUnitApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

MooseUnitApp::MooseUnitApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  srand(libMesh::processor_id());

  Moose::registerObjects(_factory);
  Moose::associateSyntax(_syntax, _action_factory);
}

MooseUnitApp::~MooseUnitApp()
{
}

