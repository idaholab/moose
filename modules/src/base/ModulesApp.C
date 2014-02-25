#include "ModulesApp.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "AppFactory.h"

/************************************************************
 * New Module Step 1.                                       *
 *         Add include for new modules here                 *
 * #if defined(MODULENAME) || defined(ALL_MODULES)          *
 *   #include "ModuleNameApp.h"                             *
 * #endif                                                   *
 ***********************************************************/
template<>
InputParameters validParams<ModulesApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

ModulesApp::ModulesApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  Moose::registerObjects(_factory);
  ModulesApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ModulesApp::associateSyntax(_syntax, _action_factory);
}

ModulesApp::~ModulesApp()
{
}

void
ModulesApp::registerApps()
{
  registerApp(ModulesApp);
}

void
ModulesApp::registerObjects(Factory & factory)
{
  /************************************************************
   * New Module Step 2.                                       *
   *                Register module objects here              *
   * ModuleNameApp::registerObjects(factory);                 *
   * if defined(MODULENAME) || defined(ALL_MODULES)           *
   *   ModulenameApp::registerObjects(factory);               *
   * #endif                                                   *
   ***********************************************************/
}

void
ModulesApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  /************************************************************
   * New Module Step 3.                                       *
   *                Associate syntax here                     *
   * #if defined(MODULENAME) || defined(ALL_MODULES)          *
   *   ModuleNameApp::associateSyntax(syntax, action_factory);*
   * #endif                                                   *
   ***********************************************************/
}
