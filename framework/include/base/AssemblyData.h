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

#ifndef ASSEMBLYDATA_H
#define ASSEMBLYDATA_H

// libMesh includes
#include "fe.h"
#include "quadrature.h"
#include "elem.h"
#include "node.h"

// MOOSE Forward Declares
class MooseMesh;
class ArbitraryQuadrature;

/**
 * Keeps track of stuff related to assembling
 *
 */
class AssemblyData
{
public:
  AssemblyData(MooseMesh & mesh);
  virtual ~AssemblyData();

  FEBase * & getFE(FEType type);

  /**
   * Returns the reference to the current quadrature being used
   */
  QBase * & qRule() { return _qrule; }

  /**
   * Returns the reference to the quadrature points
   */
  const std::vector<Point> & qPoints() { return _q_points; }

  /**
   * TODO:
   */
  const std::vector<Point> & physicalPoints() { return _current_physical_points; }

  /**
   * Returns the reference to the transformed jacobian weights
   */
  const std::vector<Real> & JxW() { return _JxW; }

  FEBase * & getFEFace(FEType type);

  /**
   * Returns the reference to the current quadrature being used on a current face
   */
  QBase * & qRuleFace() { return _qrule_face; }

  /**
   * Returns the reference to the current quadrature being used
   */
  const std::vector<Point> & qPointsFace() { return _q_points_face; }

  /**
   * Returns the reference to the transformed jacobian weights on a current face
   */
  const std::vector<Real> & JxWFace() { return _JxW_face; }

  FEBase * & getFEFaceNeighbor(FEType type);

  /**
   * Returns the array of normals for quadrature points on a current side
   */
  const std::vector<Point> & normals() { return _normals; }

  /**
   * Return the current element
   */
  const Elem * & elem() { return _current_elem; }

  /**
   * Returns the current side
   */
  unsigned int & side() { return _current_side; }

  /**
   * Returns the side element
   */
  const Elem * & sideElem() { return _current_side_elem; }

  /**
   * Return the neighbor element
   */
  const Elem * & neighbor() { return _neighbor_elem; }


  /**
   * Returns the reference to the node
   */
  const Node * & node() { return _current_node; }

  /**
   * Creates the volume, face and arbitrary qrules based on the Order passed in.
   */
  void createQRules(QuadratureType type, Order o);

  /**
   * Set the qrule to be used for volume integration.
   *
   * Note: This is normally set internally, only use if you know what you are doing!
   */
  void setVolumeQRule(QBase * qrule);

  /**
   * Set the qrule to be used for face integration.
   *
   * Note: This is normally set internally, only use if you know what you are doing!
   */
  void setFaceQRule(QBase * qrule);

  /**
   * Reinitialize objects (JxW, q_points, ...) for an elements
   *
   * @param elem The element we want to reinitialize on
   */
  void reinit(const Elem * elem);

  /**
   * Reinitialize the assembly data at specific physical point in the given element.
   */
  void reinitAtPhysical(const Elem * elem, const std::vector<Point> & physical_points);

  /**
   * Reinitialize the assembly data at specific points in the reference element.
   */
  void reinit(const Elem * elem, const std::vector<Point> & reference_points);

  /**
   * Reinitialize the assembly data on an side of an element
   */
  void reinit(const Elem * elem, unsigned int side);

  /**
   * Reinitialize element and its neighbor
   * @param elem Element being reinitialized
   * @param side Side of the element
   * @param neighbor Neighbor facing the element on the side 'side'
   */
  void reinit(const Elem * elem, unsigned int side, const Elem * neighbor);

  /**
   * Reinitializes the neighbor's face at the physical coordinates given.
   */
  void reinitNeighborAtPhysical(const Elem * neighbor, unsigned int neighbor_side, const std::vector<Point> & physical_points);

  /**
   * Reinitialize assembly data for a node
   */
  void reinit(const Node * node);

  /**
   * Compute the volume of currently selected element
   *
   * @return the volume of the element
   */
  Real computeVolume();

protected:
  MooseMesh & _mesh;

  std::map<FEType, FEBase *> _fe;               ///< types of finite elements
  FEBase * _fe_helper;                          ///< helper object for transforming coordinates
  QBase * _qrule;                               ///< current quadrature rule being used (could be either volumetric or arbitrary - for dirac kernels)
  QBase * _qrule_volume;                        ///< volumetric quadrature for the element
  ArbitraryQuadrature * _qrule_arbitrary;       ///< arbitrary quadrature rule used within the element interior
  ArbitraryQuadrature * _qface_arbitrary;       ///< arbitrary quadrature rule used on element faces

  const std::vector<Point> & _q_points;         ///< reference to the list of quadrature points
  const std::vector<Real> & _JxW;               ///< reference to the list of transformed jacobian weights

  std::map<FEType, FEBase *> _fe_face;          ///< types of finite elements
  FEBase * _fe_face_helper;                     ///< helper object for transforming coordinates
  QBase * _qrule_face;                          ///< quadrature rule used on faces
  const std::vector<Point> & _q_points_face;    ///< reference to the quadrature points on a face
  const std::vector<Real> & _JxW_face;          ///< reference to the transformed jacobian weights on a face
  const std::vector<Point> & _normals;          ///< Normal vectors at the quadrature points.

  std::map<FEType, FEBase *> _fe_neighbor;      ///< types of finite elements
  FEBase * _fe_neighbor_helper;                 ///< helper object for transforming coordinates

  const Elem * _current_elem;                   ///< The current "element" we are currently on.
  unsigned int _current_side;                   ///< The current side of the selected element (valid only when working with sides)
  const Elem * _current_side_elem;              ///< The current "element" making up the side we are currently on.
  const Elem * _neighbor_elem;                  ///< The current neighbor "element"

  const Node * _current_node;                   ///< The current node we are working with

  std::vector<Point> _current_physical_points;  ///< This will be filled up with the physical points passed into reinitAtPhysical() if it is called.  Invalid at all other times.
};

#endif /* ASSEMBLYDATA_H */
