//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"
#include "MooseMesh.h"

/**
 * This initial condition builds a data structure that is queried
 * for initial condition information
 */
class DataStructIC : public InitialCondition
{
public:
  static InputParameters validParams();

  DataStructIC(const InputParameters & parameters);
  virtual ~DataStructIC();

  virtual void initialSetup();

  virtual Real value(const Point & /*p*/);

private:
  MooseMesh & _mesh;
  std::map<dof_id_type, Real> _data;
};
