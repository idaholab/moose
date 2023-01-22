//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideUserObject.h"
#include <vector>
#include "libmesh/data_type.h"

/**
 * Given a radiation direction vector this user object computes the illumination state of each side
 * QP on the sideset it is operating on. A value of 1 indicates that the QP is receiving radiation,
 * while a value of 0 indicates it is in the shadow of another element.
 */
class SelfShadowSideUserObject : public SideUserObject
{
public:
  using SideIDType = std::pair<dof_id_type, unsigned int>;

  static InputParameters validParams();

  SelfShadowSideUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  /// only needed for ElementUserObjects and NodalUseroObjects
  virtual void threadJoin(const UserObject & y) override;

  // API to check if a QP is illuminated
  int illumination(const SideIDType & id) const;

  /// API to chek if teh UO runs on the displaced mesh
  bool useDisplacedMesh() const { return getParam<bool>("use_displaced_mesh"); }

  using Triangle = std::tuple<Point, Point, Point, SideIDType>;
  using LineSegment = std::tuple<Point, Point, SideIDType>;

protected:
  /// problem dimension
  const unsigned int _dim;

  /// raw illumination vector data (direction the radiation is propagating in)
  std::vector<const PostprocessorValue *> _raw_direction;

  /// matrix that rotates the direction onto the z-axis
  RealTensorValue _rotation;

  /// illumination status data (partition local), we use a bit for each QP
  std::map<SideIDType, unsigned int> & _illumination_status;

  /// global triange data (3D)
  std::vector<Triangle> _triangles;

  /// global line segment data (2D)
  std::vector<LineSegment> _lines;

  /// local QP data
  std::vector<std::pair<SideIDType, MooseArray<Point>>> _local_qps;

private:
  void addLines(const SideIDType & id);
  void addTriangles(const SideIDType & id);
  bool check2DIllumination(const Point & qp, const SideIDType & id);
  bool check3DIllumination(const Point & qp, const SideIDType & id);

  /// rotate all points in the given container
  void rotate(MooseArray<Point> & points);
  void rotate(Triangle & triangle);
  void rotate(LineSegment & line);
};
