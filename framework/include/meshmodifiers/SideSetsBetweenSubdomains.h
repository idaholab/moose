//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SIDESETSBETWEENSUBDOMAINS_H
#define SIDESETSBETWEENSUBDOMAINS_H

#include "MeshModifier.h"

class SideSetsBetweenSubdomains;

template <>
InputParameters validParams<SideSetsBetweenSubdomains>();

class SideSetsBetweenSubdomains : public MeshModifier
{
public:
  SideSetsBetweenSubdomains(const InputParameters & parameters);

protected:
  virtual void modify() override;
};

#endif /* SIDESETSBETWEENSUBDOMAINS_H */
