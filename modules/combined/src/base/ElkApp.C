#include "ElkApp.h"
#include "Elk.h"

template<>
InputParameters validParams<ElkApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

ElkApp::ElkApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  Moose::registerObjects(_factory);
  Elk::registerObjects(_factory);
  
  Moose::associateSyntax(_syntax, _action_factory);
  Elk::associateSyntax(_syntax, _action_factory);
}

ElkApp::~ElkApp()
{
}

