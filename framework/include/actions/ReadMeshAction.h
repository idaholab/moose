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

#ifndef READMESHACTION_H
#define READMESHACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include "MooseMesh.h"

#include <string>

class ReadMeshAction;

template<>
InputParameters validParams<ReadMeshAction>();


class ReadMeshAction : public Action
{
public:
  ReadMeshAction(const std::string & name, InputParameters params);
  
  virtual void act();

  static const std::string no_file_supplied;
};

#endif // READMESHACTION_H
