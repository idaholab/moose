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

#ifndef AUXKERNEL_H
#define AUXKERNEL_H

#include "Moose.h"
#include "MooseArray.h"
#include "PDEBase.h"
#include "MaterialPropertyInterface.h"

//forward declarations
class AuxKernel;
class MooseSystem;
class DofData;
class ElementData;
class AuxData;

template<>
InputParameters validParams<AuxKernel>();

/** 
 * AuxKernels compute values at nodes.
 */
class AuxKernel :
  public PDEBase,
  protected MaterialPropertyInterface
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  AuxKernel(const std::string & name, MooseSystem & moose_system, InputParameters parameters);

  virtual ~AuxKernel(){}

  void computeAndStore();

  bool isNodal();

protected:
  virtual Real computeValue() = 0;

  /**
   * Convenience reference to the ElementData object inside of MooseSystem
   */
  ElementData & _element_data;

  DofData & _dof_data;

  /**
   * Convenience reference to the AuxData object inside of MooseSystem
   */
  AuxData & _aux_data;

  bool _nodal;

  MooseArray<Real> & _u;
  MooseArray<Real> & _u_old;
  MooseArray<Real> & _u_older;

  virtual MooseArray<Real> & coupledValue(const std::string & name, int i = 0);
  virtual MooseArray<Real> & coupledValueOld(const std::string & name, int i = 0);
  virtual MooseArray<Real> & coupledValueOlder(const std::string & name, int i = 0);
  
  virtual MooseArray<RealGradient> & coupledGradient(const std::string & name, int i = 0);
  virtual MooseArray<RealGradient> & coupledGradientOld(const std::string & name, int i = 0);
  virtual MooseArray<RealGradient> & coupledGradientOlder(const std::string & name, int i = 0);


  /*************
   * Nodal Stuff
   *************/
  /**
   * Current Node
   */
  const Node * & _current_node;
};

#endif //AUXKERNEL_H
