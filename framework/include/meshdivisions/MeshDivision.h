//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "SetupInterface.h"
#include "Restartable.h"
#include "MeshChangedInterface.h"

#include "libmesh/vector_value.h"

// libMesh forward declarations
namespace libMesh
{
class Point;
}

namespace MooseMeshDivision
{
/// Invalid subdomain id to return when outside the mesh division
inline unsigned int INVALID_DIVISION_INDEX = std::numeric_limits<unsigned int>::max();
}

/**
 * Base class for MeshDivision objects. MeshDivision objects divide the mesh into a
 * contiguously numbered set of divisions/partitions.
 */
class MeshDivision : public MooseObject, public SetupInterface, public MeshChangedInterface
{
public:
  /**
   * Class constructor
   * \param parameters The input parameters for the MeshDivision
   */
  static InputParameters validParams();

  MeshDivision(const InputParameters & parameters);

  /**
   * MeshDivision destructor
   */
  virtual ~MeshDivision();

  /// Return the index of the division to which the point belongs
  virtual unsigned int divisionIndex(const Point & pt) const = 0;
  /// Return the index of the division to which the element belongs
  virtual unsigned int divisionIndex(const Elem & elem) const = 0;
  /// Return the number of divisions
  unsigned int getNumDivisions() const { return _num_divs; }
  /// Returns whether the entire mesh is covered by the division of the mesh, whether every point and element has a valid division index
  bool coversEntireMesh() const { return _mesh_fully_indexed; }

  /// By default, meshChanged will cause a re-initialization of the necessary data members
  virtual void meshChanged() override { initialize(); }

protected:
  /// Set the number of divisions
  void setNumDivisions(const unsigned int ndivs) { _num_divs = ndivs; }

  /// Set up any data members that would be necessary to obtain the division indices
  virtual void initialize() {}

  /// Pointer to the problem, needed to retrieve pointers to various objects
  const FEProblemBase * const _fe_problem;

  /// Mesh that is being divided
  const MooseMesh & _mesh;

  /// Whether the mesh is fully covered / indexed, all elements and points have a valid index
  bool _mesh_fully_indexed;

private:
  /// Number of divisions in the division
  unsigned int _num_divs;
};
