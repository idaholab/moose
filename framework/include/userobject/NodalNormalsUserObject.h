//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALNORMALSUSEROBJECT_H
#define NODALNORMALSUSEROBJECT_H

#include "NodalUserObject.h"

class NodalNormalsUserObject;

template <>
InputParameters validParams<NodalNormalsUserObject>();

/**
 * This user object holds nodal normals that can be used by other systems
 *
 * To compute nodal normals, we sweep through the mesh and contribute with a gradient of a shape
 * function per each boundary node.  The contribution can come from 2 different sources: (1) the
 * contribution from a node that is on a boundary, but it is not a end node of the side and (2) a
 * boundary node that is a end node.  In case (2), we are not actually contributing with a gradient
 * of a shape function, but rather the side normal, see picture below:
 *
 *     <- *---* (case 2)
 *        |   |
 *     <- *---* (case 1)
 *        |   |
 *     <- *---* (case 2)
 *
 * The normal at the top and bottom node should be horizontal, becuase that's what people would
 * intuitively picked. If we used the gradient of a shape function the normals would be pointing
 * left top and bottom left, respectively.  In order to do this, we have to treat the corner case in
 * a special way, which is done by NodalNormalsCorner user object. The contributions from the other
 * nodes are provided by NodalNormalsBoundaryNodes user object.
 *
 * The method is described in:
 * Engelman, M. S., Sani, R. L. and Gresho, P. M. (1982), The implementation of normal and/or
 * tangential boundary conditions in finite element codes for incompressible fluid flow. Int. J.
 * Numer. Meth. Fluids, 2: 225--238. doi:10.1002/fld.1650020302
 */
class NodalNormalsUserObject : public NodalUserObject
{
public:
  NodalNormalsUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;

  /**
   * Obtain a normal associated with this node ID
   *
   * @param node_id The ID of the node for which we want the normal
   * @return The normal associated with node `node_id`
   */
  virtual Point getNormal(dof_id_type node_id) const;

  /**
   * Add a contribution for a node normal
   *
   * @param node The node associated with this contribution
   * @param grad The contribution to the nodal normal
   */
  void add(const Node * node, RealGradient grad) const;

  /**
   * Zero out all computed normals.  Needed when mesh changes.
   */
  void zeroNormals() const;

  /**
   * Trigger normals computation.  This would be normally in finalize(), but we need to triggers
   * this explicitly from the user objects that accumulate the normals contribution.
   */
  void computeNormals() const;

  /**
   * Exchange normal contributions with other CPUs
   */
  void communicate() const;

protected:
  /**
   * Helper method for adding normal contributions
   *
   * @param node_id The ID of the node the normal belongs to
   * @param contr The contribution to the normal vector
   */
  void add(dof_id_type node_id, const Point & contr) const;

  /**
   * Send the contributions to the other CPUs
   *
   * @param serialized_buffer Buffer that stores tha data we sedn to other CPUs
   */
  void serialize(std::string & serialized_buffer) const;

  /**
   * Receive the contributions from other CPUs
   *
   * @param serialized_buffers Buffers for storing stream incoming from other CPUs
   */
  void deserialize(std::vector<std::string> & serialized_buffers) const;

  /// Nodal normals for nodes
  std::map<dof_id_type, Point> & _nodal_normals;

  /// Mutex that prevents multiple threads from adding normal contribution at the same time
  static Threads::spin_mutex _add_contr;
};

#endif /* NODALNORMALSUSEROBJECT_H */
