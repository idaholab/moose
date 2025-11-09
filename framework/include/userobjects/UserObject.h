//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "UserObjectBase.h"

#include "libmesh/parallel.h"

/**
 * Base class for user-specific data
 */
class UserObject : public UserObjectBase
{
public:
  static InputParameters validParams();

  UserObject(const InputParameters & params);

  /**
   * Execute method.
   */
  virtual void execute() = 0;

  /**
   * Optional interface function for "evaluating" a UserObject at a spatial position.
   * If a UserObject overrides this function that UserObject can then be used in a
   * Transfer to transfer information from one domain to another.
   */
  virtual Real spatialValue(const Point & /*p*/) const
  {
    mooseError(name(), " does not satisfy the Spatial UserObject interface!");
  }

  /**
   * Optional interface function for providing the points at which a UserObject attains
   * spatial values. If a UserObject overrides this function, then other objects that
   * take both the UserObject and points can instead directly use the points specified
   * on the UserObject.
   */
  virtual const std::vector<Point> spatialPoints() const
  {
    mooseError("Spatial UserObject interface is not satisfied; spatialPoints() must be overridden");
  }

  /**
   * Must override.
   *
   * @param uo The UserObject to be joined into _this_ object.  Take the data from the uo object and
   * "add" it into the data for this object.
   */
  virtual void threadJoin(const UserObject & uo) = 0;

  void setPrimaryThreadCopy(UserObject * primary);

  UserObject * primaryThreadCopy() { return _primary_thread_copy; }

  /**
   * Whether or not a threaded copy of this object is needed when obtaining it in
   * another object, like via the UserObjectInterface.
   *
   * Derived classes should override this as needed.
   */
  virtual bool needThreadedCopy() const { return false; }

protected:
  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;

private:
  UserObject * _primary_thread_copy = nullptr;
};
