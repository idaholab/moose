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
class DofData;

/**
 * One stop shop for all the data an AuxKernel class needs.
 *
 * _One_ of these will get built for each MooseSystem.
 */
class AuxData
{
public:
  AuxData(MooseSystem & moose_system, DofData & dof_data, ElementData & element_data);
  virtual ~AuxData();

  void init();

  void reinit(const NumericVector<Number>& soln, const Node & node);
  void reinit(const NumericVector<Number>& soln, const Elem & elem);

public:
  /**
   * The MooseSystem this Kernel is associated with.
   */
  MooseSystem & _moose_system;

  DofData & _dof_data;

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
  std::vector<MooseArray<Real> > _var_vals_nodal;

  /**
   * Value of the variables at the nodes.
   */
  std::vector<MooseArray<Real> > _var_vals_old_nodal;

  /**
   * Value of the variables at the nodes at t-2.
   */
  std::vector<MooseArray<Real> > _var_vals_older_nodal;

  /**
   * Value of the variables at the nodes.
   */
  std::vector<MooseArray<Real> > _aux_var_vals_nodal;

  /**
   * Value of the variables at the nodes.
   */
  std::vector<MooseArray<Real> > _aux_var_vals_old_nodal;

  /**
   * Value of the variables at the nodes at t-2.
   */
  std::vector<MooseArray<Real> > _aux_var_vals_older_nodal;


  /*****************
   * Elemental Stuff
   *****************/

  Real _current_volume;

  std::set<unsigned int> _element_var_nums;

  /**
   * Value of the variables at the elements.
   */
  MooseArray<MooseArray<Real> > & _var_vals_element;

  /**
   * Value of the variables at the elements.
   */
  MooseArray<MooseArray<Real> > & _var_vals_old_element;

  /**
   * Value of the variables at the elements at t-2.
   */
  MooseArray<MooseArray<Real> > & _var_vals_older_element;

  /**
   * Gradient of the variables at the elements.
   */
  MooseArray<MooseArray<RealGradient> > & _var_grads_element;

  /**
   * Gradient of the variables at the elements.
   */
  MooseArray<MooseArray<RealGradient> > & _var_grads_old_element;

  /**
   * Gradient of the variables at the elements at t-2.
   */
  MooseArray<MooseArray<RealGradient> > & _var_grads_older_element;


  /**
   * Value of the variables at the elements.
   */
  MooseArray<MooseArray<Real> > & _aux_var_vals_element;

  /**
   * Value of the variables at the elements.
   */
  MooseArray<MooseArray<Real> > & _aux_var_vals_old_element;

  /**
   * Value of the variables at the elements at t-2.
   */
  MooseArray<MooseArray<Real> > & _aux_var_vals_older_element;

  /**
   * Gradient of the variables at the elements.
   */
  MooseArray<MooseArray<RealGradient> > & _aux_var_grads_element;

  /**
   * Gradient of the variables at the elements.
   */
  MooseArray<MooseArray<RealGradient> > & _aux_var_grads_old_element;

  /**
   * Gradient of the variables at the elements at t-2.
   */
  MooseArray<MooseArray<RealGradient> > & _aux_var_grads_older_element;

};

#endif //AUXDATA_H
