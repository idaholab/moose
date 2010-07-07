#ifndef AUXDATA_H
#define AUXDATA_H

//Moose includes
#include "Moose.h"
#include "MooseArray.h"

//libMesh includes
#include "numeric_vector.h"
#include "dense_matrix.h"
#include "dense_subvector.h"

//Forward Declarations
class MooseSystem;
class ElementData;

/**
 * One stop shop for all the data an AuxKernel class needs.
 *
 * _One_ of these will get built for each MooseSystem.
 */
class AuxData
{
public:
  AuxData(MooseSystem & moose_system, ElementData & element_data);
  virtual ~AuxData();

  void sizeEverything();

  void init();

  void reinit(THREAD_ID tid, const NumericVector<Number>& soln, const Node & node);
  void reinit(THREAD_ID tid, const NumericVector<Number>& soln, const Elem & elem);

  Real integrateValueAux(const MooseArray<Real> & vals, const std::vector<Real> & JxW, const std::vector<Point> & q_point);
  RealGradient integrateGradientAux(const MooseArray<RealGradient> & grads, const std::vector<Real> & JxW, const std::vector<Point> & q_point);

public:
  /**
   * The MooseSystem this Kernel is associated with.
   */
  MooseSystem & _moose_system;

  /**
   * Element Data
   */
  ElementData & _element_data;

  const NumericVector<Number> * _nonlinear_old_soln;
  const NumericVector<Number> * _nonlinear_older_soln;

  NumericVector<Number> * _aux_soln;
  const NumericVector<Number> * _aux_old_soln;
  const NumericVector<Number> * _aux_older_soln;

  /**
   * Holds the variable numbers of the nodal aux vars.
   */
  std::set<unsigned int> _nodal_var_nums;

  /**
   * Value of the variables at the nodes.
   */
  std::vector<std::vector<MooseArray<Real> > > _var_vals_nodal;

  /**
   * Value of the variables at the nodes.
   */
  std::vector<std::vector<MooseArray<Real> > > _var_vals_old_nodal;

  /**
   * Value of the variables at the nodes at t-2.
   */
  std::vector<std::vector<MooseArray<Real> > > _var_vals_older_nodal;

  /**
   * Value of the variables at the nodes.
   */
  std::vector<std::vector<MooseArray<Real> > > _aux_var_vals_nodal;

  /**
   * Value of the variables at the nodes.
   */
  std::vector<std::vector<MooseArray<Real> > > _aux_var_vals_old_nodal;

  /**
   * Value of the variables at the nodes at t-2.
   */
  std::vector<std::vector<MooseArray<Real> > > _aux_var_vals_older_nodal;



  /*****************
   * Elemental Stuff
   *****************/

  /**
   * Holds the variable numbers of the elemental aux vars.
   */
  std::set<unsigned int> _element_var_nums;

  /**
   * Value of the variables at the elements.
   */
  std::vector<std::vector<MooseArray<Real> > > _var_vals_element;

  /**
   * Value of the variables at the elements.
   */
  std::vector<std::vector<MooseArray<Real> > > _var_vals_old_element;

  /**
   * Value of the variables at the elements at t-2.
   */
  std::vector<std::vector<MooseArray<Real> > > _var_vals_older_element;

  /**
   * Gradient of the variables at the elements.
   */
  std::vector<std::vector<MooseArray<RealGradient> > > _var_grads_element;

  /**
   * Gradient of the variables at the elements.
   */
  std::vector<std::vector<MooseArray<RealGradient> > > _var_grads_old_element;

  /**
   * Gradient of the variables at the elements at t-2.
   */
  std::vector<std::vector<MooseArray<RealGradient> > > _var_grads_older_element;

  /**
   * Value of the variables at the elements.
   */
  std::vector<std::vector<MooseArray<Real> > > _aux_var_vals_element;

  /**
   * Value of the variables at the elements.
   */
  std::vector<std::vector<MooseArray<Real> > > _aux_var_vals_old_element;

  /**
   * Value of the variables at the elements at t-2.
   */
  std::vector<std::vector<MooseArray<Real> > > _aux_var_vals_older_element;

  /**
   * Gradient of the variables at the elements.
   */
  std::vector<std::vector<MooseArray<RealGradient> > > _aux_var_grads_element;

  /**
   * Gradient of the variables at the elements.
   */
  std::vector<std::vector<MooseArray<RealGradient> > > _aux_var_grads_old_element;

  /**
   * Gradient of the variables at the elements at t-2.
   */
  std::vector<std::vector<MooseArray<RealGradient> > > _aux_var_grads_older_element;
};

#endif //AUXDATA_H
