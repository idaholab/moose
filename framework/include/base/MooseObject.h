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

#ifndef MOOSEOBJECT_H
#define MOOSEOBJECT_H

// MOOSE includes
#include "InputParameters.h"
#include "ConsoleStreamInterface.h"

// libMesh includes
#include "libmesh/parallel_object.h"


/**
 * A helper class to allow MooseObject to have the block/boundary restricted methods.
 *
 * When storing objects that are restricted it is desired to have the ability to know
 * if an object is block/boundary restricted. Since this may not be accomplished with the
 * InputParameters since they are const, a method must be available to perform this test.
 *
 * Therefore, this class which is virtually inherited by both MooseObject and BlockRestrictable
 * allows for this function to exist on every MooseObject (defaulting to false) and be overridden
 * by the Block Restricted class, in the case this method should return true.
 *
 * Additionally, performing a dynamic_cast to BoundaryRestrcitable in MooseObjectStorage is not possible due to cyclic
 * dependencies with FEProblem.h.
 *
 * @see MooseObjectStorage
 */
class BlockRestrictableHelper
{
public:
  BlockRestrictableHelper() {}
  virtual ~BlockRestrictableHelper(){}
  virtual bool isBlockRestrictable(){ return false; }
  virtual bool blockRestricted(){return false; }
  virtual const std::set<SubdomainID> & blockIDs(bool /*mesh_ids = false*/) const { mooseError("Object is not Block restrictable."); return _blk_ids; }

protected:
  std::set<SubdomainID> _blk_ids;
};


/**
 * A helper class to allow MooseObject to have the 'boundaryRestricted' method.
 * @see BlockRestrictableHelper
 */
class BoundaryRestrictableHelper
{
public:
  BoundaryRestrictableHelper(){}
  virtual ~BoundaryRestrictableHelper(){}
  virtual bool isBoundaryRestrictable(){ return false; }
  virtual bool boundaryRestricted(){return false; }
  virtual const std::set<BoundaryID> & boundaryIDs() const { mooseError("Object is not Boundary restrictable."); return _bnd_ids; }

protected:
  std::set<BoundaryID> _bnd_ids;
};


class MooseApp;
class MooseObject;

template<>
InputParameters validParams<MooseObject>();


/**
 * Every object that can be built by the factory should be derived from this class.
 */
class MooseObject :
  public virtual BlockRestrictableHelper, // see BlockRestrictable.h
  public virtual BoundaryRestrictableHelper, // see BoundaryRestrictable.h
  public ConsoleStreamInterface,
  public libMesh::ParallelObject
{
public:
  MooseObject(const InputParameters & parameters);

  virtual ~MooseObject() { }

  /**
   * Get the name of the object
   * @return The name of the object
   */
  const std::string & name() const { return _name; }

  /**
   * Get the parameters of the object
   * @return The parameters of the object
   */
  const InputParameters & parameters() const { return _pars; }

  /**
   * Retrieve a parameter for the object
   * @param name The name of the parameter
   * @return The value of the parameter
   */
  template <typename T>
  const T & getParam(const std::string & name) const;

  /**
   * Test if the supplied parameter is valid
   * @param name The name of the parameter to test
   */
  inline bool isParamValid(const std::string &name) const { return _pars.isParamValid(name); }

  /**
   * Get the MooseApp this object is associated with.
   */
  MooseApp & getMooseApp() { return _app; }

  /**
   * Return the enabled status of the object.
   */
  bool enabled() { return _enabled; }

  /**
   * Returns true of the object has been restricted to a boundary.
   * @see BoundaryRestrictable
   */
  //virtual bool boundaryRestricted() { return false; }

  /**
   * Returns true of the object has been restricted to a subdomain.
   * @see BlockRestrictable
   */
  //virtual bool blockRestricted() { return false; }


protected:

  /// The MooseApp this object is associated with
  MooseApp & _app;

  /// Parameters of this object, references the InputParameters stored in the InputParametersWarehouse
  const InputParameters & _pars;

  /// The name of this object, reference to value stored in InputParameters
  const std::string & _name;

  /// Reference to the "enable" InputParaemters, used by Controls for toggling on/off MooseObjects
  const bool & _enabled;

};

template <typename T>
const T &
MooseObject::getParam(const std::string & name) const
{
  return InputParameters::getParamHelper(name, _pars, static_cast<T *>(0));
}


#endif /* MOOSEOBJECT_H*/
