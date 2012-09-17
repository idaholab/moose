/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef GENERATEDMESH_H
#define GENERATEDMESH_H

#include "MooseMesh.h"

#include "MooseEnum.h"

class GeneratedMesh;
class NonlinearSystem;

template<>
InputParameters validParams<GeneratedMesh>();

/**
 * Mesh generated from parameters
 */
class GeneratedMesh : public MooseMesh
{
public:
  GeneratedMesh(const std::string & name, InputParameters parameters);
  virtual ~GeneratedMesh();

  /**
   * Returns the width of the requested dimension
   */
  Real dimensionWidth(unsigned int component) const;

  /**
   * Returns the min or max of the requested dimension respectively
   */
  Real getMinInDimension(unsigned int component) const;
  Real getMaxInDimension(unsigned int component) const;

  /**
   * Returns whether this generated mesh is periodic in the given dimension
   * for the given variable
   */
  bool isPeriodic(NonlinearSystem &nl, unsigned int var_num, unsigned int component);

  /**
   * This function initializes the data structures necessary for calling minPeriodicDistance.
   * @param nl - A reference to the nonlinear system
   * @param var_num - The variable inspected for periodicity
   */
  void initPeriodicDistanceForVariable(NonlinearSystem &nl, unsigned int var_num);

  /**
   * This function returns the distance between two points taking into account periodicity.  A call
   * to initPeriodicDistanceForVariable should be made prior to calling this function or it will
   * not factor in any PBCs.
   * @param p, q - The points for which to compute a minimum distance
   * @return Real - The L2 distance between p and q
   */
  Real minPeriodicDistance(Point p, Point q) const;

protected:
  /// The dimension of the mesh
  MooseEnum _dim;
  /// Number of elements in x, y, z direction
  int _nx, _ny, _nz;

  enum {X=0, Y, Z};
  enum {MIN=0, MAX};

  /// The bounds in each direction of the mesh
  std::vector<std::vector<Real> > _bounds;

  std::vector<bool> _periodic_dim;
  Point _half_range;
};

#endif /* GENERATEDMESH_H */
