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

#ifndef ELEMENTUSEROBJECT_H
#define ELEMENTUSEROBJECT_H

#include "MooseVariable.h"
#include "UserObject.h"
#include "UserObjectInterface.h"
#include "Coupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"

// libMesh
#include "elem.h"
#include "MooseTypes.h"

//Forward Declarations
class ElementUserObject;

template<>
InputParameters validParams<ElementUserObject>();

class ElementUserObject :
  public UserObject,
  public UserObjectInterface,
  public Coupleable,
  public MooseVariableDependencyInterface,
  public TransientInterface,
  protected PostprocessorInterface
{
public:
  ElementUserObject(const std::string & name, InputParameters parameters);

  const std::vector<SubdomainName> & blocks() { return _blocks; }

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
/// The block ID this postprocessor works on
  std::vector<SubdomainName> _blocks;

  /// The current element pointer (available during execute())
  const Elem * & _current_elem;

  /// The current element volume (available during execute())
  const Real & _current_elem_volume;

  const MooseArray< Point > & _q_point;
  QBase * & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  // Single Instance Variables
  Real & _real_zero;
  MooseArray<Real> & _zero;
  MooseArray<RealGradient> & _grad_zero;
  MooseArray<RealTensor> & _second_zero;
};

#endif
