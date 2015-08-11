/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LINEAR_ELASTICITYAPP_H
#define LINEAR_ELASTICITYAPP_H

#include "MooseApp.h"

class LinearElasticityApp;

template<>
InputParameters validParams<LinearElasticityApp>();

class LinearElasticityApp : public MooseApp
{
public:
  LinearElasticityApp(const InputParameters & parameters);
  virtual ~LinearElasticityApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* LINEAR_ELASTICITYAPP_H */
