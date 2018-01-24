/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EBSDMESH_H
#define EBSDMESH_H

#include "GeneratedMesh.h"

#include <array>

class EBSDMesh;

template <>
InputParameters validParams<EBSDMesh>();

/**
 * Mesh generated from parameters
 */
class EBSDMesh : public GeneratedMesh
{
public:
  EBSDMesh(const InputParameters & parameters);
  virtual ~EBSDMesh();

  virtual void buildMesh();

  struct EBSDMeshGeometry
  {
    // grid spacing
    std::array<Real, 3> d;
    // grid origin
    std::array<Real, 3> min;
    // mesh dimension
    unsigned int dim;
    // grid size
    std::array<unsigned int, 3> n;
  };

  // Interface functions for the EBSDReader
  const EBSDMeshGeometry & getEBSDGeometry() const { return _geometry; }
  const std::string & getEBSDFilename() const { return _filename; }

protected:
  /// Read the EBSD data file header
  void readEBSDHeader();

  /// Name of the file containing the EBSD data
  std::string _filename;

  /// EBSD data file mesh information
  EBSDMeshGeometry _geometry;
};

#endif // EBSDMESH_H
