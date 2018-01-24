//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ASSIGNELEMENTSUBDOMAINID_H
#define ASSIGNELEMENTSUBDOMAINID_H

// MOOSE includes
#include "MeshModifier.h"

// Forward declerations
class AssignElementSubdomainID;

template <>
InputParameters validParams<AssignElementSubdomainID>();

/**
 * MeshModifier for assigning subdomain IDs of all elements
 */
class AssignElementSubdomainID : public MeshModifier
{
public:
  AssignElementSubdomainID(const InputParameters & parameters);

protected:
  virtual void modify() override;
};

#endif // ASSIGNELEMENTSUBDOMAINID_H
