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

#ifndef SETUPMESHACTION_H
#define SETUPMESHACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include "MooseMesh.h"

#include <string>

class SetupMeshAction : public Action
{
public:
  SetupMeshAction(const std::string & name, InputParameters params);

  MooseMesh * createMesh(bool save_exreader);
  
  virtual void act();

  static const std::string no_file_supplied;
};

template<>
InputParameters validParams<SetupMeshAction>();

#endif // SETUPMESHACTION_H
