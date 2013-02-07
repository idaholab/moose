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

#ifndef BNDELEMENT_H
#define BNDELEMENT_H

#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"
#include "libmesh/stored_range.h"
#include "libmesh/variant_filter_iterator.h"

struct BndElement
{
  BndElement(Elem * elem, unsigned short int side, BoundaryID bnd_id) :
      _elem(elem),
      _side(side),
      _bnd_id(bnd_id)
  {
  }

  /// pointer to the element
  Elem * _elem;
  /// side number
  unsigned short int _side;
  /// boundary id for the node
  BoundaryID _bnd_id;
};

/**
 * The definition of the element_iterator struct.
 */
struct
bnd_elem_iterator :
variant_filter_iterator<MeshBase::Predicate,
                        BndElement*>
{
  // Templated forwarding ctor -- forwards to appropriate variant_filter_iterator ctor
  template <typename PredType, typename IterType>
  bnd_elem_iterator (const IterType& d,
                     const IterType& e,
                     const PredType& p ) :
    variant_filter_iterator<MeshBase::Predicate,
                            BndElement*>(d,e,p) {}
};




/**
 * The definition of the const_element_iterator struct.  It is similar to the regular
 * iterator above, but also provides an additional conversion-to-const ctor.
 */
struct
const_bnd_elem_iterator :
variant_filter_iterator<MeshBase::Predicate,
                        BndElement* const,
                        BndElement* const&,
                        BndElement* const*>
{
  // Templated forwarding ctor -- forwards to appropriate variant_filter_iterator ctor
  template <typename PredType, typename IterType>
  const_bnd_elem_iterator (const IterType& d,
                           const IterType& e,
                           const PredType& p ) :
    variant_filter_iterator<MeshBase::Predicate,
                            BndElement* const,
                            BndElement* const&,
                            BndElement* const*>(d,e,p)  {}


  // The conversion-to-const ctor.  Takes a regular iterator and calls the appropriate
  // variant_filter_iterator copy constructor.  Note that this one is *not* templated!
  const_bnd_elem_iterator (const bnd_elem_iterator& rhs) :
    variant_filter_iterator<MeshBase::Predicate,
                            BndElement* const,
                            BndElement* const&,
                            BndElement* const*>(rhs)
  {
    // libMesh::out << "Called element_iterator conversion-to-const ctor." << std::endl;
  }
};


typedef StoredRange<bnd_elem_iterator,             BndElement*>      BndElemRange;
typedef StoredRange<const_bnd_elem_iterator, const BndElement*> ConstBndElemRange;



#endif /* BNDELEMENT_H */
