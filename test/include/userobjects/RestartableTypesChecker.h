//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RESTARTABLETYPESCHECKER_H
#define RESTARTABLETYPESCHECKER_H

#include "RestartableTypes.h"

class RestartableTypesChecker;

template <>
InputParameters validParams<RestartableTypesChecker>();

/**
 * User Object for testing Restartable data types
 */
class RestartableTypesChecker : public RestartableTypes
{
public:
  RestartableTypesChecker(const InputParameters & parameters);
  virtual ~RestartableTypesChecker();

  virtual void initialSetup();
  virtual void timestepSetup();

  virtual void initialize(){};
  virtual void execute();
  virtual void finalize(){};

  void checkData();
  void clearTypes();
};

#endif /* RESTARTABLETYPESCHECKER_H */
