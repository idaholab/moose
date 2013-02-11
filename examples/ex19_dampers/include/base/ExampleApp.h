#ifndef EXAMPLEAPP_H
#define EXAMPLEAPP_H

#include "MooseApp.h"

class ExampleApp : public MooseApp
{
public:
  ExampleApp(int argc, char * argv[]);
  virtual ~ExampleApp();
};

#endif /* EXAMPLEAPP_H */
