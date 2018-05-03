//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FILEMESHPD_H
#define FILEMESHPD_H

#include "MeshBasePD.h"

class FileMeshPD;

template <>
InputParameters validParams<FileMeshPD>();

/**
 * Mesh class for creating peridynamic mesh from EXODUS finite element mesh
 */
class FileMeshPD : public MeshBasePD
{
public:
  FileMeshPD(const InputParameters & parameters);
  virtual ~FileMeshPD();

  virtual std::unique_ptr<MooseMesh> safeClone() const override;
  virtual void init() override;
  virtual void buildMesh() override;

  /// EXODUS finite element mesh
  MeshBase * _fe_mesh;
};

#endif /* FILEMESHPD_H */
