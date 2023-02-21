//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredBase.h"

// MOOSE includes
#include "MooseEnum.h"
#include "MooseMesh.h"
#include "SubProblem.h"
#include "UserObject.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/point.h"

InputParameters
LayeredBase::validParams()
{
  InputParameters params = emptyInputParameters();
  MooseEnum directions("x y z");

  params.addRequiredParam<MooseEnum>("direction", directions, "The direction of the layers.");
  params.addParam<unsigned int>("num_layers", "The number of layers.");
  params.addParam<std::vector<Real>>("bounds",
                                     "The 'bounding' positions of the layers i.e.: '0, "
                                     "1.2, 3.7, 4.2' will mean 3 layers between those "
                                     "positions.");

  MooseEnum sample_options("direct interpolate average", "direct");
  params.addParam<MooseEnum>("sample_type",
                             sample_options,
                             "How to sample the layers.  'direct' means get the value of the layer "
                             "the point falls in directly (or average if that layer has no value). "
                             " 'interpolate' does a linear interpolation between the two closest "
                             "layers.  'average' averages the two closest layers.");

  params.addParam<unsigned int>("average_radius",
                                1,
                                "When using 'average' sampling this is how "
                                "the number of values both above and below "
                                "the layer that will be averaged.");

  params.addParam<bool>(
      "cumulative",
      false,
      "When true the value in each layer is the sum of the values up to and including that layer");
  params.addParam<bool>(
      "positive_cumulative_direction",
      true,
      "When 'cumulative' is true, whether the direction for summing the cumulative value "
      "is the positive direction or negative direction");

  params.addParam<std::vector<SubdomainName>>(
      "block", "The list of block ids (SubdomainID) that this object will be applied");

  params.addParam<std::vector<SubdomainName>>("layer_bounding_block",
                                              "List of block ids (SubdomainID) that are used to "
                                              "determine the upper and lower geometric bounds for "
                                              "all layers. If this is not specified, the ids "
                                              "specified in 'block' are used for this purpose.");

  params.addParam<Real>("direction_min",
                        "Minimum coordinate along 'direction' that bounds the layers");
  params.addParam<Real>("direction_max",
                        "Maximum coordinate along 'direction' that bounds the layers");
  params.addParamNamesToGroup("direction num_layers bounds direction_min direction_max",
                              "Layers extent and definition");
  params.addParamNamesToGroup("sample_type average_radius cumulative positive_cumulative_direction",
                              "Value sampling / aggregating");
  return params;
}

LayeredBase::LayeredBase(const InputParameters & parameters)
  : Restartable(parameters.getCheckedPointerParam<SubProblem *>("_subproblem")->getMooseApp(),
                parameters.get<std::string>("_object_name") + "_layered_base",
                "LayeredBase",
                parameters.get<THREAD_ID>("_tid")),
    _layered_base_name(parameters.get<std::string>("_object_name")),
    _layered_base_params(parameters),
    _direction_enum(parameters.get<MooseEnum>("direction")),
    _direction(_direction_enum),
    _sample_type(parameters.get<MooseEnum>("sample_type")),
    _average_radius(parameters.get<unsigned int>("average_radius")),
    _using_displaced_mesh(_layered_base_params.get<bool>("use_displaced_mesh")),
    _layer_values(declareRestartableData<std::vector<Real>>("layer_values")),
    _layer_has_value(declareRestartableData<std::vector<int>>("layer_has_value")),
    _layered_base_subproblem(*parameters.getCheckedPointerParam<SubProblem *>("_subproblem")),
    _cumulative(parameters.get<bool>("cumulative")),
    _positive_cumulative_direction(parameters.get<bool>("positive_cumulative_direction")),
    _layer_bounding_blocks(),
    _has_direction_max_min(false)
{
  if (_layered_base_params.isParamValid("num_layers") &&
      _layered_base_params.isParamValid("bounds"))
    mooseError("'bounds' and 'num_layers' cannot both be set");

  if (!_cumulative && parameters.isParamSetByUser("positive_cumulative_direction"))
    mooseWarning(
        "The 'positive_cumulative_direction' parameter is unused when 'cumulative' is false");

  if (_layered_base_params.isParamValid("num_layers"))
  {
    _num_layers = _layered_base_params.get<unsigned int>("num_layers");
    _interval_based = true;
  }
  else if (_layered_base_params.isParamValid("bounds"))
  {
    _interval_based = false;

    _layer_bounds = _layered_base_params.get<std::vector<Real>>("bounds");

    // Make sure the bounds are sorted - we're going to depend on this
    std::sort(_layer_bounds.begin(), _layer_bounds.end());

    _num_layers = _layer_bounds.size() - 1; // Layers are only in-between the bounds
    _direction_min = _layer_bounds.front();
    _direction_max = _layer_bounds.back();
    _has_direction_max_min = true;
  }
  else
    mooseError("One of 'bounds' or 'num_layers' must be specified");

  if (!_interval_based && _sample_type == 1)
    mooseError("'sample_type = interpolate' not supported with 'bounds'");

  bool has_layer_bounding_block = _layered_base_params.isParamValid("layer_bounding_block");
  bool has_block = _layered_base_params.isParamValid("block");
  bool has_direction_min = _layered_base_params.isParamValid("direction_min");
  bool has_direction_max = _layered_base_params.isParamValid("direction_max");

  if (_has_direction_max_min && has_direction_min)
    mooseWarning("'direction_min' is unused when providing 'bounds'");

  if (_has_direction_max_min && has_direction_max)
    mooseWarning("'direction_max' is unused when providing 'bounds'");

  // can only specify one of layer_bounding_block or the pair direction_max/min
  if (has_layer_bounding_block && (has_direction_min || has_direction_max))
    mooseError("Only one of 'layer_bounding_block' and the pair 'direction_max' and "
               "'direction_min' can be provided");

  // if either one of direction_min or direction_max is specified, must provide the other one
  if (has_direction_min != has_direction_max)
    mooseError("If providing the layer max/min directions, both 'direction_max' and "
               "'direction_min' must be specified.");

  if (has_layer_bounding_block)
    _layer_bounding_blocks = _layered_base_subproblem.mesh().getSubdomainIDs(
        _layered_base_params.get<std::vector<SubdomainName>>("layer_bounding_block"));
  else if (has_block)
    _layer_bounding_blocks = _layered_base_subproblem.mesh().getSubdomainIDs(
        _layered_base_params.get<std::vector<SubdomainName>>("block"));

  // specifying the direction max/min overrides anything set with the 'block'
  if (has_direction_min && has_direction_max)
  {
    _direction_min = parameters.get<Real>("direction_min");
    _direction_max = parameters.get<Real>("direction_max");
    _has_direction_max_min = true;

    if (_direction_max <= _direction_min)
      mooseError("'direction_max' must be larger than 'direction_min'");
  }

  _layer_values.resize(_num_layers);
  _layer_has_value.resize(_num_layers);

  // if we haven't already figured out the max/min in specified direction
  // (either with the 'bounds' or explicit specification from the user), do so
  if (!_has_direction_max_min)
    getBounds();

  computeLayerCenters();
}

Real
LayeredBase::integralValue(Point p) const
{
  unsigned int layer = getLayer(p);

  int higher_layer = -1;
  int lower_layer = -1;

  for (unsigned int i = layer; i < _layer_values.size(); i++)
  {
    if (_layer_has_value[i])
    {
      higher_layer = i;
      break;
    }
  }

  for (int i = layer - 1; i >= 0; i--)
  {
    if (_layer_has_value[i])
    {
      lower_layer = i;
      break;
    }
  }

  if (higher_layer == -1 && lower_layer == -1)
    return 0; // TODO: We could error here but there are startup dependency problems

  switch (_sample_type)
  {
    case 0: // direct
    {
      if (higher_layer == -1) // Didn't find a higher layer
        return _layer_values[lower_layer];

      if (unsigned(higher_layer) == layer) // constant in a layer
        return _layer_values[higher_layer];

      if (lower_layer == -1) // Didn't find a lower layer
        return _layer_values[higher_layer];

      return (_layer_values[higher_layer] + _layer_values[lower_layer]) / 2;
    }
    case 1: // interpolate
    {
      if (higher_layer == -1) // Didn't find a higher layer
        return _layer_values[lower_layer];

      Real layer_length = (_direction_max - _direction_min) / _num_layers;
      Real lower_coor = _direction_min;
      Real lower_value = 0;
      if (lower_layer != -1)
      {
        lower_coor += (lower_layer + 1) * layer_length;
        lower_value = _layer_values[lower_layer];
      }

      // Interpolate between the two points
      Real higher_value = _layer_values[higher_layer];

      // Linear interpolation
      return lower_value +
             (higher_value - lower_value) * (p(_direction) - lower_coor) / layer_length;
    }
    case 2: // average
    {
      Real total = 0;
      unsigned int num_values = 0;

      if (higher_layer != -1)
      {
        for (unsigned int i = 0; i < _average_radius; i++)
        {
          int current_layer = higher_layer + i;

          if ((size_t)current_layer >= _layer_values.size())
            break;

          if (_layer_has_value[current_layer])
          {
            total += _layer_values[current_layer];
            num_values += 1;
          }
        }
      }

      if (lower_layer != -1)
      {
        for (unsigned int i = 0; i < _average_radius; i++)
        {
          int current_layer = lower_layer - i;

          if (current_layer < 0)
            break;

          if (_layer_has_value[current_layer])
          {
            total += _layer_values[current_layer];
            num_values += 1;
          }
        }
      }

      return total / num_values;
    }
    default:
      mooseError("Unknown sample type!");
  }
}

Real
LayeredBase::getLayerValue(unsigned int layer) const
{
  if (layer >= _layer_values.size())
    mooseError("Layer '", layer, "' not found in '", _layered_base_name, "'.");
  return _layer_values[layer];
}

void
LayeredBase::initialize()
{
  if (_using_displaced_mesh)
    getBounds();

  for (unsigned int i = 0; i < _layer_values.size(); i++)
  {
    _layer_values[i] = 0.0;
    _layer_has_value[i] = false;
  }
}

void
LayeredBase::finalize()
{
  _layered_base_subproblem.comm().sum(_layer_values);
  _layered_base_subproblem.comm().max(_layer_has_value);

  if (_cumulative)
  {
    Real value = 0;

    if (_positive_cumulative_direction)
      for (unsigned i = 0; i < _num_layers; i++)
      {
        value += getLayerValue(i);
        setLayerValue(i, value);
      }
    else
      for (int i = _num_layers - 1; i >= 0; i--)
      {
        value += getLayerValue(i);
        setLayerValue(i, value);
      }
  }
}

void
LayeredBase::threadJoin(const UserObject & y)
{
  const LayeredBase & lb = dynamic_cast<const LayeredBase &>(y);
  for (unsigned int i = 0; i < _layer_values.size(); i++)
    if (lb.layerHasValue(i))
      setLayerValue(i, getLayerValue(i) + lb._layer_values[i]);
}

unsigned int
LayeredBase::getLayer(Point p) const
{
  Real direction_x = p(_direction);

  if (direction_x < _direction_min)
    return 0;

  if (_interval_based)
  {
    unsigned int layer =
        std::floor(((direction_x - _direction_min) / (_direction_max - _direction_min)) *
                   static_cast<Real>(_num_layers));

    if (layer >= _num_layers)
      layer = _num_layers - 1;

    return layer;
  }
  else // Figure out what layer we are in from the bounds
  {
    // This finds the first entry in the vector that is larger than what we're looking for
    std::vector<Real>::const_iterator one_higher =
        std::upper_bound(_layer_bounds.begin(), _layer_bounds.end(), direction_x);

    if (one_higher == _layer_bounds.end())
    {
      return static_cast<unsigned int>(
          _layer_bounds.size() -
          2); // Just return the last layer.  -2 because layers are "in-between" bounds
    }
    else if (one_higher == _layer_bounds.begin())
      return 0; // Return the first layer
    else
      // The -1 is because the interval that we fall in is just _before_ the number that is bigger
      // (which is what we found
      return static_cast<unsigned int>(std::distance(_layer_bounds.begin(), one_higher - 1));
  }
}

void
LayeredBase::computeLayerCenters()
{
  _layer_centers.resize(_num_layers);

  if (_interval_based)
  {
    Real dx = (_direction_max - _direction_min) / _num_layers;

    for (unsigned int i = 0; i < _num_layers; ++i)
      _layer_centers[i] = (i + 0.5) * dx;
  }
  else
  {
    for (unsigned int i = 0; i < _num_layers; ++i)
      _layer_centers[i] = 0.5 * (_layer_bounds[i + 1] + _layer_bounds[i]);
  }
}

void
LayeredBase::setLayerValue(unsigned int layer, Real value)
{
  _layer_values[layer] = value;
  _layer_has_value[layer] = true;
}

void
LayeredBase::getBounds()
{
  if (_layer_bounding_blocks.size() == 0)
  {
    BoundingBox bounding_box = MeshTools::create_bounding_box(_layered_base_subproblem.mesh());
    _direction_min = bounding_box.min()(_direction);
    _direction_max = bounding_box.max()(_direction);
  }
  else
  {
    _direction_min = std::numeric_limits<Real>::infinity();
    _direction_max = -std::numeric_limits<Real>::infinity();

    MooseMesh & mesh = _layered_base_subproblem.mesh();

    for (auto & elem_ptr : *mesh.getActiveLocalElementRange())
    {
      auto subdomain_id = elem_ptr->subdomain_id();

      if (std::find(_layer_bounding_blocks.begin(), _layer_bounding_blocks.end(), subdomain_id) ==
          _layer_bounding_blocks.end())
        continue;

      for (auto & node : elem_ptr->node_ref_range())
      {
        _direction_min = std::min(_direction_min, node(_direction));
        _direction_max = std::max(_direction_max, node(_direction));
      }
    }

    mesh.comm().min(_direction_min);
    mesh.comm().max(_direction_max);
  }
}
