#ifndef EXAMPLEAPP_H
#define EXAMPLEAPP_H

#include "MooseApp.h"

class ExampleApp;

template<>
InputParameters validParams<ExampleApp>();

class ExampleApp : public MooseApp
{
public:
  ExampleApp(const std::string & name, InputParameters parameters);
  virtual ~ExampleApp();
};

#endif /* EXAMPLEAPP_H */
