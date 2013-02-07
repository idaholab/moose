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

#ifndef BNDNODE_H
#define BNDNODE_H

#include "libmesh/node.h"
#include "libmesh/mesh_base.h"
#include "libmesh/stored_range.h"
#include "libmesh/variant_filter_iterator.h"

struct BndNode
{
  BndNode(Node * node, BoundaryID bnd_id) :
      _node(node),
      _bnd_id(bnd_id)
  {
  }

  /// pointer to the node
  Node * _node;
  /// boundary id for the node
  BoundaryID _bnd_id;
};

/**
 * The definition of the element_iterator struct.
 */
struct
bnd_node_iterator :
variant_filter_iterator<MeshBase::Predicate,
                        BndNode*>
{
  // Templated forwarding ctor -- forwards to appropriate variant_filter_iterator ctor
  template <typename PredType, typename IterType>
  bnd_node_iterator (const IterType& d,
                    const IterType& e,
                    const PredType& p ) :
    variant_filter_iterator<MeshBase::Predicate,
                            BndNode*>(d,e,p) {}
};




/**
 * The definition of the const_element_iterator struct.  It is similar to the regular
 * iterator above, but also provides an additional conversion-to-const ctor.
 */
struct
const_bnd_node_iterator :
variant_filter_iterator<MeshBase::Predicate,
                        BndNode* const,
                        BndNode* const&,
                        BndNode* const*>
{
  // Templated forwarding ctor -- forwards to appropriate variant_filter_iterator ctor
  template <typename PredType, typename IterType>
  const_bnd_node_iterator (const IterType& d,
                           const IterType& e,
                           const PredType& p ) :
    variant_filter_iterator<MeshBase::Predicate,
                            BndNode* const,
                            BndNode* const&,
                            BndNode* const*>(d,e,p)  {}


  // The conversion-to-const ctor.  Takes a regular iterator and calls the appropriate
  // variant_filter_iterator copy constructor.  Note that this one is *not* templated!
  const_bnd_node_iterator (const bnd_node_iterator& rhs) :
    variant_filter_iterator<MeshBase::Predicate,
                            BndNode* const,
                            BndNode* const&,
                            BndNode* const*>(rhs)
  {
    // libMesh::out << "Called element_iterator conversion-to-const ctor." << std::endl;
  }
};


typedef StoredRange<bnd_node_iterator,             BndNode*>      BndNodeRange;
typedef StoredRange<const_bnd_node_iterator, const BndNode*> ConstBndNodeRange;



#endif /* BNDNODE_H */
