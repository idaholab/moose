//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ASSIGNSUBDOMAINID_H
#define ASSIGNSUBDOMAINID_H

// MOOSE includes
#include "MeshModifier.h"

// Forward declerations
class AssignSubdomainID;

template <>
InputParameters validParams<AssignSubdomainID>();

/**
 * MeshModifier for assigning a subdomain ID to all elements
 */
class AssignSubdomainID : public MeshModifier
{
public:
  AssignSubdomainID(const InputParameters & parameters);

protected:
  virtual void modify() override;

  /// The subdomain ID to assign to every elemennt
  SubdomainID _subdomain_id;
};

#endif // ASSIGNSUBDOMAINID_H
