//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENTUOIC_H
#define ELEMENTUOIC_H

#include "InitialCondition.h"
#include "MooseMesh.h"

class ElementUOIC;
class ElementUOProvider;

template <>
InputParameters validParams<ElementUOIC>();

/**
 * Initial Condition for returing values from an ElementUOProvider derived class.
 */
class ElementUOIC : public InitialCondition
{
public:
  ElementUOIC(const InputParameters & parameters);

  virtual Real value(const Point & /*p*/);

private:
  MooseMesh & _mesh;
  std::map<dof_id_type, Real> _data;

  const ElementUOProvider & _elem_uo;
  const std::string _field_name;
  const MooseEnum _field_type;
};

#endif /* ELEMENTUOIC_H */
