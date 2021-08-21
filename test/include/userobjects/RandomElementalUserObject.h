//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"

/**
 * An Elemental user object that uses built-in Random number generation.
 */
class RandomElementalUserObject : public ElementUserObject
{
public:
  static InputParameters validParams();

  RandomElementalUserObject(const InputParameters & parameters);

  virtual ~RandomElementalUserObject();

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & y);
  virtual void finalize();

  unsigned long getElementalValue(unsigned int element_id) const;

protected:
  std::map<dof_id_type, unsigned long> _random_data;
};
