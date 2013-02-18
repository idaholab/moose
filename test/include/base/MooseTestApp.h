#ifndef MOOSETESTAPP_H
#define MOOSETESTAPP_H

#include "MooseApp.h"

class MooseTestApp;

template<>
InputParameters validParams<MooseTestApp>();

class MooseTestApp : public MooseApp
{
public:
  MooseTestApp(const std::string & name, InputParameters parameters);

  virtual ~MooseTestApp();
};

#endif /* MOOSETESTAPP_H */
