//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BCUserObject.h"
#include "ElementLoopUserObject.h"

using libMesh::RealGradient;

/**
 * Base class for piecewise linear slope reconstruction
 * to get the slopes of element average variables
 */
class SlopeReconstructionBase : public ElementLoopUserObject
{
public:
  static InputParameters validParams();

  SlopeReconstructionBase(const InputParameters & parameters);

  virtual void initialize();
  virtual void finalize();

  virtual void computeElement();

  /// accessor function call to get element slope values
  virtual const std::vector<RealGradient> & getElementSlope(dof_id_type elementid) const;

  /// accessor function call to get element average variable values
  virtual const std::vector<Real> & getElementAverageValue(dof_id_type elementid) const;

  /// accessor function call to get boundary average variable values
  virtual const std::vector<Real> & getBoundaryAverageValue(dof_id_type elementid,
                                                            unsigned int side) const;

  /// accessor function call to get cached internal side centroid
  virtual const Point & getSideCentroid(dof_id_type elementid, dof_id_type neighborid) const;

  /// accessor function call to get cached boundary side centroid
  virtual const Point & getBoundarySideCentroid(dof_id_type elementid, unsigned int side) const;

  /// accessor function call to get cached internal side normal
  virtual const Point & getSideNormal(dof_id_type elementid, dof_id_type neighborid) const;

  /// accessor function call to get cached boundary side centroid
  virtual const Point & getBoundarySideNormal(dof_id_type elementid, unsigned int side) const;

  /// accessor function call to get cached internal side area
  virtual const Real & getSideArea(dof_id_type elementid, dof_id_type neighborid) const;

  /// accessor function call to get cached boundary side area
  virtual const Real & getBoundarySideArea(dof_id_type elementid, unsigned int side) const;

  /// compute the slope of the cell
  virtual void reconstructElementSlope() = 0;

  virtual void meshChanged();

protected:
  virtual void serialize(std::string & serialized_buffer);
  virtual void deserialize(std::vector<std::string> & serialized_buffers);

  /// store the reconstructed slopes into this map indexed by element ID
  std::map<dof_id_type, std::vector<RealGradient>> & _rslope;

  /// store the average variable values into this map indexed by element ID
  std::map<dof_id_type, std::vector<Real>> & _avars;

  /// store the boundary average variable values into this map indexed by pair of element ID and local side ID
  std::map<std::pair<dof_id_type, unsigned int>, std::vector<Real>> & _bnd_avars;

  /// store the side centroid into this map indexed by pair of element ID and neighbor ID
  std::map<std::pair<dof_id_type, dof_id_type>, Point> & _side_centroid;

  /// store the boundary side centroid into this map indexed by pair of element ID and local side ID
  std::map<std::pair<dof_id_type, unsigned int>, Point> & _bnd_side_centroid;

  /// store the side area into this map indexed by pair of element ID and neighbor ID
  std::map<std::pair<dof_id_type, dof_id_type>, Real> & _side_area;

  /// store the boundary side area into this map indexed by pair of element ID and local side ID
  std::map<std::pair<dof_id_type, unsigned int>, Real> & _bnd_side_area;

  /// store the side normal into this map indexed by pair of element ID and neighbor ID
  std::map<std::pair<dof_id_type, dof_id_type>, Point> & _side_normal;

  /// store the boundary side normal into this map indexed by pair of element ID and local side ID
  std::map<std::pair<dof_id_type, unsigned int>, Point> & _bnd_side_normal;

  /// required data for face assembly
  const MooseArray<Point> & _q_point_face;
  const QBase * const & _qrule_face;
  const MooseArray<Real> & _JxW_face;
  const MooseArray<Point> & _normals_face;

  /// current side of the current element
  const unsigned int & _side;

  const Elem * const & _side_elem;
  const Real & _side_volume;

  /// the neighboring element
  const Elem * const & _neighbor_elem;

  /// flag to indicated if side geometry info is cached
  bool _side_geoinfo_cached;

private:
  static Threads::spin_mutex _mutex;
};
