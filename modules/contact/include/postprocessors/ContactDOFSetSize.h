//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

// Forward Declarations
class MooseVariableFieldBase;
namespace libMesh
{
class MeshBase;
}

class ContactDOFSetSize : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  ContactDOFSetSize(const InputParameters & parameters);

  void initialize() override;
  void execute() override;

  PostprocessorValue getValue() override;

private:
  /// MOOSE variable we compute the contact set from
  const MooseVariableFieldBase & _var;

  /// The libmesh mesh
  const MeshBase & _mesh;

  /// The subdomain over which to query degrees of freedom
  const SubdomainID _subdomain_id;

  /// The tolerance used to decide whether the variable indicates contact
  const Real _tolerance;

  /// Represents the number of values in contact
  unsigned int _count;
};
