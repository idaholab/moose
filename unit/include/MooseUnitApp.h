#ifndef MOOSEUNITAPP_H
#define MOOSEUNITAPP_H

#include "MooseApp.h"

class MooseUnitApp;

template<>
InputParameters validParams<MooseUnitApp>();

class MooseUnitApp : public MooseApp
{
public:
  MooseUnitApp(const std::string & name, InputParameters parameters);
  virtual ~MooseUnitApp();
};

#endif /* MOOSEUNITAPP_H */
