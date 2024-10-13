//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementLoopUserObject.h"

using libMesh::RealGradient;

/**
 * Base class for slope limiting to limit
 * the slopes of cell average variables
 */
class SlopeLimitingBase : public ElementLoopUserObject
{
public:
  static InputParameters validParams();

  SlopeLimitingBase(const InputParameters & parameters);

  virtual void initialize();
  virtual void finalize();

  virtual void computeElement();

  /// accessor function call
  virtual const std::vector<RealGradient> & getElementSlope(dof_id_type elementid) const;

  /// compute the slope of the cell
  virtual std::vector<RealGradient> limitElementSlope() const = 0;

protected:
  virtual void serialize(std::string & serialized_buffer);
  virtual void deserialize(std::vector<std::string> & serialized_buffers);

  /// store the updated slopes into this map indexed by element ID
  std::map<dof_id_type, std::vector<RealGradient>> & _lslope;

  /// option whether to include BCs
  const bool _include_bc;

  /// required data for face assembly
  const MooseArray<Point> & _q_point_face;
  const libMesh::QBase * const & _qrule_face;
  const MooseArray<Real> & _JxW_face;
  const MooseArray<Point> & _normals_face;

  /// current side of the current element
  const unsigned int & _side;

  const Elem * const & _side_elem;
  const Real & _side_volume;

  /// the neighboring element
  const Elem * const & _neighbor_elem;

private:
  static Threads::spin_mutex _mutex;
};
