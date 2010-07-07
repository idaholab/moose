#ifndef FACEDATA_H
#define FACEDATA_H

#include "Moose.h"
#include "MooseArray.h"
#include "QuadraturePointData.h"

//Forward Declarations
class MooseSystem;

template <class T> class NumericVector;

class FaceData : public QuadraturePointData
{
public:
  FaceData(MooseSystem & moose_system);
  virtual ~FaceData();

  void sizeEverything();

  void init();

  void reinit(THREAD_ID tid, const NumericVector<Number>& soln, const Elem * elem, const unsigned int side, const unsigned int boundary_id);
  void reinit(THREAD_ID tid, const NumericVector<Number>& soln, const Node & node, const unsigned int boundary_id, NumericVector<Number>& residual);

public:
  /**
   * The MooseSystem this Kernel is associated with.
   */
  MooseSystem & _moose_system;

  /// BCs
  /**
   * Current node for nodal BC's
   */
  std::vector<const Node *> _current_node;

  /**
   * Current residual vector.  Only valid for nodal BC's.
   */
  std::vector<NumericVector<Number> *> _current_residual;

  /**
   * Current side.
   */
  std::vector<unsigned int> _current_side;


  /**
   * Normal vectors at the quadrature points.
   */
  std::vector<std::map<FEType, const std::vector<Point> *> > _normals;

  /**
   * Map to vector of variable numbers that need to be evaluated
   * at the nodes on that boundary
   */
  std::map<unsigned int, std::set<unsigned int> > _boundary_to_var_nums_nodal;

  /**
   * Holds the current dof numbers for each variable for nodal bcs
   */
  std::vector<std::vector<unsigned int> > _nodal_bc_var_dofs;

  /**
   * Value of the variables at the nodes.
   */
  MooseArray<MooseArray<MooseArray<Real> > > _var_vals_nodal;
};


#endif //FACEDATA_H
