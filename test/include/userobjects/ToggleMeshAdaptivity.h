/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef TOGGLEMESHADAPTIVITY_H
#define TOGGLEMESHADAPTIVITY_H

#include "GeneralUserObject.h"

class ToggleMeshAdaptivity;

template <>
InputParameters validParams<ToggleMeshAdaptivity>();

class ToggleMeshAdaptivity : public GeneralUserObject
{
public:
  ToggleMeshAdaptivity(const InputParameters & params);

  virtual void initialSetup();

  virtual void initialize(){};
  virtual void execute();
  virtual void finalize(){};

protected:
  void checkState();

  MooseEnum _state;

  int _steps_to_wait;
};

#endif /* TOGGLEMESHADAPTIVITY_H */
