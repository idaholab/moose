//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CAVITYPRESSUREUOACTION_H
#define CAVITYPRESSUREUOACTION_H

#include "Action.h"
#include "MooseTypes.h"

class CavityPressureUOAction : public Action
{
public:
  CavityPressureUOAction(const InputParameters & params);

  virtual void act();

  const Real _initial_pressure;
  const std::vector<PostprocessorName> _material_input;
  const Real _R;
  const PostprocessorName _temperature;
  const PostprocessorName _volume;
  const Real _startup_time;
};

template <>
InputParameters validParams<CavityPressureUOAction>();

#endif
