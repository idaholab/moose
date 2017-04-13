/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef SLOPELIMITINGBASE_H
#define SLOPELIMITINGBASE_H

#include "ElementLoopUserObject.h"
#include "SlopeReconstructionBase.h"

// Forward Declarations
class SlopeLimitingBase;

template <>
InputParameters validParams<SlopeLimitingBase>();

/**
 * Base class for slope limiting to limit
 * the slopes of cell average variables
 */
class SlopeLimitingBase : public ElementLoopUserObject
{
public:
  SlopeLimitingBase(const InputParameters & parameters);

  virtual void initialize();
  virtual void threadJoin(const UserObject & y);
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
  std::map<dof_id_type, std::vector<RealGradient>> _lslope;

  /// option whether to include BCs
  bool _include_bc;

  /// slope reconstruction user object
  const SlopeReconstructionBase & _rslope;

  /// required data for face assembly
  const MooseArray<Point> & _q_point_face;
  QBase *& _qrule_face;
  const MooseArray<Real> & _JxW_face;
  const MooseArray<Point> & _normals_face;

  /// current side of the current element
  unsigned int & _side;

  const Elem *& _side_elem;
  const Real & _side_volume;

  /// the neighboring element
  const Elem *& _neighbor_elem;

private:
  static Threads::spin_mutex _mutex;
};

#endif
