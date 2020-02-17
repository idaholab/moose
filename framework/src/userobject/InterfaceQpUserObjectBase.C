//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceQpUserObjectBase.h"

InputParameters
InterfaceQpUserObjectBase::validParams()
{
  InputParameters params = InterfaceValueUserObject::validParams();
  params.addClassDescription("Base class to compute a scalar value or rate across an interface");
  params.addParam<MooseEnum>("value_type",
                             InterfaceQpUserObjectBase::valueOptions(),
                             "Type of value to compute and store");
  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  return params;
}

InterfaceQpUserObjectBase::InterfaceQpUserObjectBase(const InputParameters & parameters)
  : InterfaceValueUserObject(parameters), _value_type(getParam<MooseEnum>("value_type"))

{
}

void
InterfaceQpUserObjectBase::initialSetup()
{
  // define the boundary map and retrieve element side and boundary_ID
  std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> elem_side_bid =
      _mesh.buildSideList();

  // retrieve on which boundary this UO operates
  std::set<BoundaryID> boundaryList = boundaryIDs();

  // initialize the map_values looping over all the element and sides
  for (unsigned int i = 0; i < elem_side_bid.size(); i++)
  {
    // check if this element side is part of the boundary, if so add element side to the interface
    // map
    if (boundaryList.find(std::get<2>(elem_side_bid[i])) != boundaryList.end())
    {
      // make pair
      std::pair<dof_id_type, unsigned int> elem_side_pair =
          std::make_pair(std::get<0>(elem_side_bid[i]), std::get<1>(elem_side_bid[i]));
      // initialize map elemenet
      std::vector<Real> var_values(0, 0);

      // add entry to the value map
      _map_values[elem_side_pair] = var_values;
      _map_JxW[elem_side_pair] = var_values;
    }
  }
}

void
InterfaceQpUserObjectBase::execute()
{
  // find the entry on the map
  auto it = _map_values.find(std::make_pair(_current_elem->id(), _current_side));
  if (it != _map_values.end())
  {
    // insert two vector value for each qp
    auto & vec = _map_values[std::make_pair(_current_elem->id(), _current_side)];
    vec.resize(_qrule->n_points());
    auto & jxw = _map_JxW[std::make_pair(_current_elem->id(), _current_side)];
    jxw.resize(_qrule->n_points());

    // loop over qps and do stuff
    for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    {
      // compute average value at qp
      vec[qp] = computeRealValue(qp);
      jxw[qp] = _JxW[qp];
    }
  }
  else
    mooseError("InterfaceQpUserObjectBase:: cannot find the required element and side");
}

Real
InterfaceQpUserObjectBase::getQpValue(const dof_id_type elem,
                                      const unsigned int side,
                                      const unsigned int qp) const
{
  auto data = _map_values.find(std::make_pair(elem, side));
  if (data != _map_values.end())
    return data->second[qp];
  else
    mooseError("getQpValue: can't find the given qp");
}

Real
InterfaceQpUserObjectBase::getSideAverageValue(const dof_id_type elem,
                                               const unsigned int side) const
{
  auto data = _map_values.find(std::make_pair(elem, side));
  if (data == _map_values.end())
    mooseError("getSideAverageValue: can't find the given qp");
  auto weights = _map_JxW.find(std::make_pair(elem, side));
  if (weights == _map_JxW.end())
    mooseError("getSideAverageValue: can't find the given qp");

  Real vol = 0;
  Real val = 0;
  for (unsigned int i = 0; i < data->second.size(); i++)
  {
    val += data->second[i] * weights->second[i];
    vol += weights->second[i];
  }
  return val / vol;
}
