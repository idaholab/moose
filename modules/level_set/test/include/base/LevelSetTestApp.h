#ifndef LEVELSETTESTAPP_H
#define LEVELSETTESTAPP_H

#include "LevelSetApp.h"

class LevelSetTestApp;

template <>
InputParameters validParams<LevelSetTestApp>();

class LevelSetTestApp : public LevelSetApp
{
public:
  LevelSetTestApp(InputParameters parameters);
  virtual ~LevelSetTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* LEVELSETTESTAPP_H */
