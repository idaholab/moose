//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ElementIntegralVariableUserObject.h"
#include "Enumerate.h"
#include "DelimitedFileReader.h"
#include "LayeredBase.h"

// Forward Declarations
class UserObject;

/**
 * This UserObject computes averages of a variable storing partial
 * sums for the specified number of intervals in a direction (x,y,z).
 *
 * Given a list of points this object computes the layered average
 * closest to each one of those points.
 */
template <typename UserObjectType, typename BaseType>
class NearestPointBase : public BaseType
{
public:
  static InputParameters validParams();

  NearestPointBase(const InputParameters & parameters);
  ~NearestPointBase();

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

  /**
   * Given a Point return the integral value associated with the layer
   * that point falls in for the layered average closest to that
   * point.
   *
   * @param p The point to look for in the layers.
   */
  virtual Real spatialValue(const Point & p) const override;

  /**
   * Get the points at which the nearest operation is performed
   * @return points
   */
  virtual const std::vector<Point> & getPoints() const { return _points; }

  virtual const std::vector<Point> spatialPoints() const override;

protected:
  /**
   * Fills in the `_points` variable from either 'points' or 'points_file' parameter.
   * Also performs error checking.
   */
  void fillPoints();

  /**
   * Get the UserObject that is closest to the point.
   *
   * @param p The point.
   * @return The UserObject closest to p.
   */
  std::shared_ptr<UserObjectType> nearestUserObject(const Point & p) const;

  std::vector<Point> _points;
  std::vector<std::shared_ptr<UserObjectType>> _user_objects;

  // To specify whether the distance is defined based on point or radius
  const unsigned int _dist_norm;
  // The axis around which the radius is determined
  const unsigned int _axis;

  // The list of InputParameter objects. This is a list because these cannot be copied (or moved).
  std::list<InputParameters> _sub_params;

  using BaseType::_communicator;
  using BaseType::_current_elem;
  using BaseType::isParamValid;
  using BaseType::name;
  using BaseType::processor_id;
};

template <typename UserObjectType, typename BaseType>
InputParameters
NearestPointBase<UserObjectType, BaseType>::validParams()
{
  InputParameters params = BaseType::validParams();

  params.addParam<std::vector<Point>>("points",
                                      "Computations will be lumped into values at these points.");
  params.addParam<FileName>("points_file",
                            "A filename that should be looked in for points. Each "
                            "set of 3 values in that file will represent a Point. "
                            "This and 'points' cannot be both supplied.");

  MooseEnum distnorm("point=0 radius=1", "point");
  params.addParam<MooseEnum>(
      "dist_norm", distnorm, "To specify whether the distance is defined based on point or radius");
  MooseEnum axis("x=0 y=1 z=2", "z");
  params.addParam<MooseEnum>("axis", axis, "The axis around which the radius is determined");

  params.addParamNamesToGroup("points points_file dist_norm axis", "Points and distance to points");

  // Add in the valid parameters
  params += UserObjectType::validParams();

  return params;
}

template <typename UserObjectType, typename BaseType>
NearestPointBase<UserObjectType, BaseType>::NearestPointBase(const InputParameters & parameters)
  : BaseType(parameters),
    _dist_norm(this->template getParam<MooseEnum>("dist_norm")),
    _axis(this->template getParam<MooseEnum>("axis"))
{
  if (this->template getParam<MooseEnum>("dist_norm") != "radius" &&
      parameters.isParamSetByUser("axis"))
    this->template paramError("axis",
                              "'axis' should only be set if 'dist_norm' is set to 'radius'");

  fillPoints();

  _user_objects.reserve(_points.size());

  // Build each of the UserObject objects:
  for (MooseIndex(_points) i = 0; i < _points.size(); ++i)
  {
    auto sub_params = emptyInputParameters();
    sub_params += parameters;
    sub_params.set<std::string>("_object_name") = name() + "_sub" + std::to_string(i);

    _sub_params.push_back(sub_params);
    _user_objects.emplace_back(std::make_shared<UserObjectType>(_sub_params.back()));
  }
}

template <typename UserObjectType, typename BaseType>
NearestPointBase<UserObjectType, BaseType>::~NearestPointBase()
{
}

template <typename UserObjectType, typename BaseType>
void
NearestPointBase<UserObjectType, BaseType>::fillPoints()
{
  if (isParamValid("points") && isParamValid("points_file"))
    mooseError(name(), ": Both 'points' and 'points_file' cannot be specified simultaneously.");

  if (isParamValid("points"))
  {
    _points = this->template getParam<std::vector<Point>>("points");
  }
  else if (isParamValid("points_file"))
  {
    const FileName & points_file = this->template getParam<FileName>("points_file");

    MooseUtils::DelimitedFileReader file(points_file, &_communicator);
    file.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
    file.read();
    _points = file.getDataAsPoints();
  }
  else
    mooseError(name(), ": You need to supply either 'points' or 'points_file' parameter.");
}

template <typename UserObjectType, typename BaseType>
void
NearestPointBase<UserObjectType, BaseType>::initialize()
{
  for (auto & user_object : _user_objects)
    user_object->initialize();
}

template <typename UserObjectType, typename BaseType>
void
NearestPointBase<UserObjectType, BaseType>::execute()
{
  nearestUserObject(_current_elem->vertex_average())->execute();
}

template <typename UserObjectType, typename BaseType>
void
NearestPointBase<UserObjectType, BaseType>::finalize()
{
  for (auto & user_object : _user_objects)
    user_object->finalize();
}

template <typename UserObjectType, typename BaseType>
void
NearestPointBase<UserObjectType, BaseType>::threadJoin(const UserObject & y)
{
  auto & npla = static_cast<const NearestPointBase &>(y);

  for (MooseIndex(_user_objects) i = 0; i < _user_objects.size(); ++i)
    _user_objects[i]->threadJoin(*npla._user_objects[i]);
}

template <typename UserObjectType, typename BaseType>
Real
NearestPointBase<UserObjectType, BaseType>::spatialValue(const Point & p) const
{
  return nearestUserObject(p)->spatialValue(p);
}

template <typename UserObjectType, typename BaseType>
std::shared_ptr<UserObjectType>
NearestPointBase<UserObjectType, BaseType>::nearestUserObject(const Point & p) const
{
  unsigned int closest = 0;
  Real closest_distance = std::numeric_limits<Real>::max();

  for (auto it : Moose::enumerate(_points))
  {
    const auto & current_point = it.value();

    Real current_distance;
    if (_dist_norm == 0)
      // the distance is computed using standard norm
      current_distance = (p - current_point).norm();
    else
    {
      // the distance is to be computed based on radii
      // in that case, we need to determine the 2 coordinate indices
      // that define the radius
      unsigned int i = 0;
      unsigned int j = 1;

      if (_axis == 0)
        i = 2;
      else if (_axis == 1)
        j = 2;

      current_distance = std::abs(
          std::sqrt(p(i) * p(i) + p(j) * p(j)) -
          std::sqrt(current_point(i) * current_point(i) + current_point(j) * current_point(j)));
    }

    if (current_distance < closest_distance)
    {
      closest_distance = current_distance;
      closest = it.index();
    }
  }

  return _user_objects[closest];
}

template <typename UserObjectType, typename BaseType>
const std::vector<Point>
NearestPointBase<UserObjectType, BaseType>::spatialPoints() const
{
  std::vector<Point> points;

  for (MooseIndex(_points) i = 0; i < _points.size(); ++i)
  {
    std::shared_ptr<LayeredBase> layered_base =
        std::dynamic_pointer_cast<LayeredBase>(_user_objects[i]);
    if (layered_base)
    {
      const auto & layers = layered_base->getLayerCenters();
      auto direction = layered_base->direction();

      for (const auto & l : layers)
      {
        Point pt = _points[i];
        pt(direction) = l;
        points.push_back(pt);
      }
    }
  }

  return points;
}
