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

#ifndef NEARESTPOINTBASE_H
#define NEARESTPOINTBASE_H

// MOOSE includes
#include "ElementIntegralVariableUserObject.h"

// Forward Declarations
class UserObject;

/**
 * Because this is a templated base class and template partial
 * specializations are not allowed... this class instead defines
 * a new templated function that is templated on the type of
 * UserObject that will be at each nearest point.
 *
 * If you inherit from this class... then call this function
 * to start your parameters for the new class
 */
template<typename UserObjectType>
InputParameters nearestPointBaseValidParams()
{
  InputParameters params = validParams<ElementIntegralVariableUserObject>();

  params.addRequiredParam<std::vector<Real> >("points", "Computations will be lumped into values at these points.");

  // Add in the valid parameters
  params += validParams<UserObjectType>();

  return params;
}


/**
 * This UserObject computes averages of a variable storing partial
 * sums for the specified number of intervals in a direction (x,y,z).
 *
 * Given a list of points this object computes the layered average
 * closest to each one of those points.
 */
template<typename UserObjectType>
class NearestPointBase : public ElementIntegralVariableUserObject
{
public:
  NearestPointBase(const InputParameters & parameters);
  ~NearestPointBase();

  virtual void initialize();
  virtual void execute();
  virtual void finalize();
  virtual void threadJoin(const UserObject & y);

  /**
   * Given a Point return the integral value associated with the layer
   * that point falls in for the layered average closest to that
   * point.
   *
   * @param p The point to look for in the layers.
   */
  virtual Real spatialValue(const Point & p) const;

protected:
  /**
   * Get the UserObject that is closest to the point.
   *
   * @param p The point.
   * @return The UserObject closest to p.
   */
  UserObjectType * nearestUserObject(const Point & p) const;

  std::vector<Point> _points;
  std::vector<UserObjectType *> _user_objects;
};


template<typename UserObjectType>
NearestPointBase<UserObjectType>::NearestPointBase(const InputParameters & parameters) :
    ElementIntegralVariableUserObject(parameters)
{
  const std::vector<Real> & points_vec = getParam<std::vector<Real> >("points");

  {
    unsigned int num_vec_entries = points_vec.size();

    mooseAssert(num_vec_entries % LIBMESH_DIM == 0, "Wrong number of entries in 'points'");

    _points.reserve(num_vec_entries / LIBMESH_DIM);

    // Read the points out of the vector
    for (unsigned int i=0; i<num_vec_entries; i+=3)
      _points.push_back(Point(points_vec[i], points_vec[i+1], points_vec[i+2]));
  }

  _user_objects.reserve(_points.size());

  // Build each of the UserObject objects:
  for (unsigned int i=0; i<_points.size(); i++)
    _user_objects.push_back(new UserObjectType(parameters));
}

template<typename UserObjectType>
NearestPointBase<UserObjectType>::~NearestPointBase()
{
  for (unsigned int i=0; i<_user_objects.size(); i++)
    delete _user_objects[i];
}

template<typename UserObjectType>
void
NearestPointBase<UserObjectType>::initialize()
{
  for (unsigned int i=0; i<_user_objects.size(); i++)
    _user_objects[i]->initialize();
}

template<typename UserObjectType>
void
NearestPointBase<UserObjectType>::execute()
{
  nearestUserObject(_current_elem->centroid())->execute();
}

template<typename UserObjectType>
void
NearestPointBase<UserObjectType>::finalize()
{
  for (unsigned int i=0; i<_user_objects.size(); i++)
    _user_objects[i]->finalize();
}

template<typename UserObjectType>
void
NearestPointBase<UserObjectType>::threadJoin(const UserObject & y)
{
  const NearestPointBase & npla = static_cast<const NearestPointBase &>(y);

  for (unsigned int i=0; i<_user_objects.size(); i++)
    _user_objects[i]->threadJoin(*npla._user_objects[i]);
}

template<typename UserObjectType>
Real
NearestPointBase<UserObjectType>::spatialValue(const Point & p) const
{
  return nearestUserObject(p)->spatialValue(p);
}

template<typename UserObjectType>
UserObjectType *
NearestPointBase<UserObjectType>::nearestUserObject(const Point & p) const
{
  unsigned int closest = 0;
  Real closest_distance = std::numeric_limits<Real>::max();

  for (unsigned int i=0; i<_points.size(); i++)
  {
    const Point & current_point = _points[i];

    Real current_distance = (p - current_point).norm();

    if (current_distance < closest_distance)
    {
      closest_distance = current_distance;
      closest = i;
    }
  }

  return _user_objects[closest];
}


#endif
