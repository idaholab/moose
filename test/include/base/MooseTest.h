#ifndef MOOSETEST_H
#define MOOSETEST_H

#include "MooseApp.h"

class MooseTestApp : public MooseApp
{
public:
  MooseTestApp(int argc, char *argv[]);

protected:
  void associateSyntax();

public:
  static void registerObjects();
};

#endif /* MOOSETEST_H */
