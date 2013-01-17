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

#ifndef SIDEUSEROBJECT_H
#define SIDEUSEROBJECT_H

#include "UserObject.h"
#include "Coupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "MooseVariable.h"
#include "TransientInterface.h"
#include "UserObjectInterface.h"
#include "MaterialPropertyInterface.h"
#include "PostprocessorInterface.h"

//Forward Declarations
class SideUserObject;

template<>
InputParameters validParams<SideUserObject>();

class SideUserObject :
  public UserObject,
  public Coupleable,
  public MooseVariableDependencyInterface,
  public UserObjectInterface,
  public TransientInterface,
  public MaterialPropertyInterface,
  protected PostprocessorInterface
{
public:
  SideUserObject(const std::string & name, InputParameters parameters);

  const std::vector<BoundaryName> & boundaries() { return _boundaries; }

  /**
   * This function will get called on each geometric object this postprocessor acts on
   * (ie Elements, Sides or Nodes).  This will most likely get called multiple times
   * before getValue() is called.
   *
   * Someone somewhere has to override this.
   */
  virtual void execute() = 0;

  /**
   * Must override.
   *
   * @param uo The UserObject to be joined into _this_ object.  Take the data from the uo object and "add" it into the data for this object.
   */
  virtual void threadJoin(const UserObject & uo) = 0;

protected:

//  MooseVariable & _var;

  /// list of variables when working on more than one
//  std::vector<MooseVariable *> _vars;

  std::vector<BoundaryName> _boundaries;

  const MooseArray< Point > & _q_point;
  QBase * & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;
//  const MooseArray<Point> & _normals;

  const Elem * & _current_elem;
  /// current side of the current element
  unsigned int & _current_side;

  const Elem * & _current_side_elem;
  const Real & _current_side_volume;

  // unknown
//  const VariableValue & _u;
//  const VariableGradient & _grad_u;

  // Single Instance Variables
  Real & _real_zero;
  MooseArray<Real> & _zero;
  MooseArray<RealGradient> & _grad_zero;
  MooseArray<RealTensor> & _second_zero;

  virtual Real computeQpIntegral() = 0;
};

#endif
