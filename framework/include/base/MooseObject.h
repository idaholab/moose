#ifndef MOOSEOBJECT_H
#define MOOSEOBJECT_H

#include <vector>
#include <string>

#include "Moose.h"
#include "InputParameters.h"

class MooseSystem;

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

protected:
  /**
   * This Object's name.
   */
  std::string _name;

  /**
   * Reference to the MooseSystem that this Object is associated to
   */
  MooseSystem &_moose_system;

  /**
   * Holds parameters for derived classes so they can be built with common constructor.
   */
  InputParameters _parameters;

  // FIXME: remove this after fixing the threading model?
  /**
   * The thread id this object is associated with.
   */
  THREAD_ID _tid;
};

#endif // MOOSEOBJECT_H
