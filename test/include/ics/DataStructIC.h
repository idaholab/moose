//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DATASTRUCTIC_H
#define DATASTRUCTIC_H

#include "InitialCondition.h"
#include "MooseMesh.h"

class DataStructIC;

template <>
InputParameters validParams<DataStructIC>();

/**
 * This initial condition builds a data structure that is queried
 * for initial condition information
 */
class DataStructIC : public InitialCondition
{
public:
  DataStructIC(const InputParameters & parameters);
  virtual ~DataStructIC();

  virtual void initialSetup();

  virtual Real value(const Point & /*p*/);

private:
  MooseMesh & _mesh;
  std::map<dof_id_type, Real> _data;
};

#endif /* DATASTRUCTIC_H */
