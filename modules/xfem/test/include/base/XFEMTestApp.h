#ifndef XFEMTESTAPP_H
#define XFEMTESTAPP_H

#include "XFEMApp.h"

class XFEMTestApp;

template <>
InputParameters validParams<XFEMTestApp>();

class XFEMTestApp : public XFEMApp
{
public:
  XFEMTestApp(InputParameters parameters);
  virtual ~XFEMTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* XFEMTESTAPP_H */
