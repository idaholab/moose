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

class MooseVariableFieldBase;
namespace libMesh
{
class MeshBase;
}

class GreaterThanLessThanPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  GreaterThanLessThanPostprocessor(const InputParameters & parameters);

  void initialize() override;
  void execute() override;

  virtual void finalize() override;
  PostprocessorValue getValue() override;

private:
  /// MOOSE variable we compute the contact set from
  const MooseVariableFEBase & _var;

  /// The libmesh mesh
  const MeshBase & _mesh;

  /// Whether we are subdomain restricting the active set search
  const bool _subdomain_restricted;

  /// An optional subdomain over which to query degrees of freedom
  const SubdomainID _subdomain_id;

  /// The tolerance used to decide whether the variable indicates contact
  const Real _value;

  /// Represents the number of values in contact
  unsigned int _count;

  /// The comparison to perform
  const MooseEnum _comparator;
};
