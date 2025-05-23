//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FileOutput.h"
#include "SurrogateModelInterface.h"

/**
 * Output object for saving SurrorateModel data to a file.
 */
class SurrogateTrainerOutput : public FileOutput, public SurrogateModelInterface
{
public:
  static InputParameters validParams();
  SurrogateTrainerOutput(const InputParameters & parameters);

protected:
  virtual void output() override;

private:
  /// List of supplied SurrogateModel objects
  const std::vector<UserObjectName> & _trainers;
};
