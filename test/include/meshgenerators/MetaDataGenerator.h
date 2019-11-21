//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneratedMeshGenerator.h"
#include "DataIO.h"
#include "MooseApp.h"

class MetaDataGenerator : public GeneratedMeshGenerator
{
public:
  static InputParameters validParams();

  MetaDataGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  class MeshData
  {
  public:
    void construct(const MeshBase & mesh, unsigned int d);
    unsigned int getMap(const Elem * elem) const { return _modded_id.find(elem)->second; }
    unsigned int mod;

  protected:
    std::map<const Elem *, unsigned int> _modded_id;
  };

protected:
  MeshData & _data;
};

template <>
void dataStore(std::ostream & stream, MetaDataGenerator::MeshData & d, void * context);
template <>
void dataLoad(std::istream & stream, MetaDataGenerator::MeshData & d, void * context);
