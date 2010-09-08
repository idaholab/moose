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

#ifndef FACEDATA_H
#define FACEDATA_H

#include "Moose.h"
#include "MooseArray.h"
#include "QuadraturePointData.h"

//Forward Declarations
class MooseSystem;
class DofData;

namespace libMesh
{
  template <class T> class NumericVector;
}

class FaceData : public QuadraturePointData
{
public:
  FaceData(MooseSystem & moose_system, DofData & dof_data);
  virtual ~FaceData();

  void init();

  void reinit(const NumericVector<Number>& soln, const Elem * elem, const unsigned int side, const unsigned int boundary_id);
  void reinit(const NumericVector<Number>& soln, const Node & node, const unsigned int boundary_id, NumericVector<Number>& residual);

  void reinitMaterials(std::vector<Material *> & materials, unsigned int side);

public:
  /**
   * The MooseSystem this Kernel is associated with.
   */
  MooseSystem & _moose_system;

  DofData & _dof_data;

  /// BCs
  /**
   * Current node for nodal BC's
   */
  const Node * _current_node;

  /**
   * Current residual vector.  Only valid for nodal BC's.
   */
  NumericVector<Number> * _current_residual;

  /**
   * Current side.
   */
  unsigned int _current_side;

  /**
   * The current "element" making up the side we are currently on.
   */
  const Elem * _current_side_elem;

  /**
   * Normal vectors at the quadrature points.
   */
  std::map<FEType, const std::vector<Point> *> _normals;

  /**
   * Map to vector of variable numbers that need to be evaluated
   * at the nodes on that boundary
   */
  std::map<unsigned int, std::set<unsigned int> > _boundary_to_var_nums_nodal;

  /**
   * Holds the current dof numbers for each variable for nodal bcs
   */
  std::vector<unsigned int> _nodal_bc_var_dofs;

  /**
   * Value of the variables at the nodes.
   */
  MooseArray<MooseArray<Real> > _var_vals_nodal;
};


#endif //FACEDATA_H
