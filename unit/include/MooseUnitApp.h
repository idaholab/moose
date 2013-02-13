#ifndef MOOSEUNITAPP_H
#define MOOSEUNITAPP_H

#include "MooseApp.h"

class MooseUnitApp : public MooseApp
{
public:
  MooseUnitApp(int argc, char * argv[]);
  virtual ~MooseUnitApp();
};

#endif /* MOOSEUNITAPP_H */
