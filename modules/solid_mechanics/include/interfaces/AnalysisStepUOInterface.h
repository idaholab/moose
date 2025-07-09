//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <string>

class FEProblemBase;
class AnalysisStepUserObject;

/**
 *
 * Interface class for step user object. It meets the requirement of getting *one* step user
 * object among all existing system user objects.
 **/
class AnalysisStepUOInterface
{
protected:
  virtual void getAnalysisStepUserObject(const FEProblemBase & fe_problem,
                                 const AnalysisStepUserObject *& step_user_object,
                                 const std::string & name);
};
