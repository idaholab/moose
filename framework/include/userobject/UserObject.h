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

#ifndef USEROBJECT_H
#define USEROBJECT_H

//MOOSE includes
#include "MooseObject.h"
#include "SetupInterface.h"
#include "MaterialPropertyInterface.h"
#include "ParallelUniqueId.h"
#include "SubProblem.h"
#include "Restartable.h"
#include "MooseMesh.h"

//libMesh includes
#include "libmesh/libmesh_common.h"
#include "libmesh/parallel.h"

class UserObject;
class FEProblem;

template<>
InputParameters validParams<UserObject>();

/**
 * Base class for user-specific data
 */
class UserObject :
  public MooseObject,
  public SetupInterface,
  public MaterialPropertyInterface,
  public Restartable
{
public:
  UserObject(const std::string & name, InputParameters params);
  virtual ~UserObject();

  // materials
  template<typename T>
  MaterialProperty<T> & getMaterialProperty(const std::string & name);

  /**
   * Called before execute() is ever called so that data can be cleared.
   */
  virtual void initialize() = 0;

  /**
   * Finalize.  This is called _after_ execute() and _after_ threadJoin()!  This is probably where you want to do MPI communication!
   */
  virtual void finalize() = 0;

  /**
   * Load user data object from a stream
   * @param stream Stream to load from
   */
  virtual void load(std::ifstream & stream);

  /**
   * Store user data object to a stream
   * @param stream Stream to store to
   */
  virtual void store(std::ofstream & stream);

  /**
   * Returns a reference to the subproblem that
   * this postprocessor is tied to
   */
  SubProblem & getSubProblem() const { return _subproblem; }

  /**
   * Optional interface function for "evaluating" a UserObject at a spatial position.
   * If a UserObject overrides this function that UserObject can then be used in a
   * Transfer to transfer information from one domain to another.
   */
  virtual Real spatialValue(const Point & /*p*/) const { mooseError(_name << " does not satisfy the Spatial UserObject interface!"); }

  /**
   * Gather the parallel sum of the variable passed in. It HAS to take care of values across all threads and CPUs (we DO hybrid parallelism!)
   *
   * After calling this, the variable that was passed in will hold the gathered value.
   */
  template <typename T>
  void gatherSum(T & value)
  {
    Parallel::sum(value);
  }

  template <typename T>
  void gatherMax(T & value)
  {
    Parallel::max(value);
  }

  template <typename T>
  void gatherMin(T & value)
  {
    Parallel::min(value);
  }

  template <typename T1, typename T2>
  void gatherProxyValueMax(T1 & value, T2 & proxy)
  {
    unsigned int rank;
    Parallel::maxloc(value, rank);
    Parallel::broadcast(proxy, rank);
  }

protected:
  SubProblem & _subproblem;
  FEProblem & _fe_problem;

  /// Thread ID of this postprocessor
  THREAD_ID _tid;
  Assembly & _assembly;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;
};

template<typename T>
MaterialProperty<T> &
UserObject::getMaterialProperty(const std::string & name)
{
  bool checked = false;
  if (parameters().isParamValid("block"))
  {
    // check blocks where the uo is defined
    std::vector<SubdomainName> blocks = parameters().get<std::vector<SubdomainName> >("block");
    if (std::find(blocks.begin(), blocks.end(), "ANY_BLOCK_ID") == blocks.end())
    {
      for (std::vector<SubdomainName>::iterator it = blocks.begin(); it != blocks.end(); ++it)
        _subproblem.delayedCheckMatProp(_subproblem.mesh().getSubdomainID(*it), name);
      checked = true;
    }
  }
  if (!checked)
  {
    // no uo blocks specified, check all blocks that are in the mesh
    const std::set<SubdomainID> & blocks = _subproblem.mesh().meshSubdomains();
    for (std::set<SubdomainID>::const_iterator it = blocks.begin(); it != blocks.end(); ++it)
      _subproblem.delayedCheckMatProp(*it, name);
  }

  return MaterialPropertyInterface::getMaterialProperty<T>(name);
}

#endif /* USEROBJECT_H */
