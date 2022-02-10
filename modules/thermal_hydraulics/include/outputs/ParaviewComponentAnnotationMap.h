//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FileOutput.h"

/**
 * Output annotation map for paraview
 *
 * The map is a JSON file that can be loaded in paraview to see the mapping from block color to
 * component name
 *
 * Note: the annotation map works with paraview 5.8+
 */
class ParaviewComponentAnnotationMap : public FileOutput
{
public:
  ParaviewComponentAnnotationMap(const InputParameters & parameters);

protected:
  virtual void output(const ExecFlagType & type);
  virtual std::string filename();

public:
  static InputParameters validParams();
};
