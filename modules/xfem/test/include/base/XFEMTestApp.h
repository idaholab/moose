#ifndef XFEMTESTAPP_H
#define XFEMTESTAPP_H

#include "MooseApp.h"

class XFEMTestApp;

template <>
InputParameters validParams<XFEMTestApp>();

class XFEMTestApp : public MooseApp
{
public:
  XFEMTestApp(InputParameters parameters);
  virtual ~XFEMTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  virtual void registerExecFlags() override;
};

#endif /* XFEMTESTAPP_H */
