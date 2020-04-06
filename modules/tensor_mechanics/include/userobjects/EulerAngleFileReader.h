//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "EulerAngleProvider.h"
#include <vector>

// Forward declaration

/**
 * Read a set of Euler angles from a file
 */
class EulerAngleFileReader : public EulerAngleProvider
{
public:
  static InputParameters validParams();

  EulerAngleFileReader(const InputParameters & parameters);

  virtual const EulerAngles & getEulerAngles(unsigned int) const;
  virtual unsigned int getGrainNum() const;

  virtual void initialize() {}
  virtual void execute() {}
  virtual void finalize() {}

protected:
  void readFile();

  FileName _file_name;
  std::vector<EulerAngles> _angles;
};
