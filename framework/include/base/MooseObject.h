#ifndef MOOSEOBJECT_H
#define MOOSEOBJECT_H

#include <vector>
#include <string>

#include "Moose.h"
#include "InputParameters.h"

class MooseSystem;
class MooseObject;
class GlobalParamsBlock;

template<>
InputParameters validParams<MooseObject>();

/**
 * Base class for MOOSE objects
 */
class MooseObject
{
public:
  MooseObject(std::string name, MooseSystem &moose_system, InputParameters parameters);
  virtual ~MooseObject();

  virtual const std::string &name();

  /**
   * Return the thread id this kernel is associated with.
   */
  THREAD_ID tid();

  template <typename T>
  inline 
  const T & getParam(const std::string &name)
  {
    return _parameters.get<T>(name);
  }

protected:
  /**
   * This Object's name.
   */
  std::string _name;

  /**
   * Reference to the MooseSystem that this Object is associated to
   */
  MooseSystem &_moose_system;

  // FIXME: remove this after fixing the threading model?
  /**
   * The thread id this object is associated with.
   */
  THREAD_ID _tid;

  /**
   * Whether or not this object should be evaluated on the displaced mesh.
   */
  bool _use_displaced_mesh;

private:
  /**
   * Holds parameters for derived classes so they can be built with common constructor.
   */
  InputParameters _parameters;

};

#endif // MOOSEOBJECT_H
