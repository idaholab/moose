//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalGravity.h"
#include "MooseUtils.h"
#include "DelimitedFileReader.h"
#include "Function.h"

registerMooseObject("TensorMechanicsApp", NodalGravity);

InputParameters
NodalGravity::validParams()
{
  InputParameters params = NodalKernel::validParams();
  params.addClassDescription("Computes the gravitational force for a given nodal mass.");
  params.addRangeCheckedParam<Real>("alpha",
                                    0.0,
                                    "alpha >= -0.3333 & alpha <= 0.0",
                                    "Alpha parameter for mass dependent numerical damping induced "
                                    "by HHT time integration scheme");
  params.addParam<Real>("mass", "Mass associated with the node");
  params.addParam<FileName>(
      "nodal_mass_file",
      "The file containing the nodal positions and the corresponding nodal masses.");
  params.addParam<Real>("gravity_value", 0.0, "Acceleration due to gravity.");
  // A ConstantFunction of "1" is supplied as the default
  params.addParam<FunctionName>(
      "function", "1", "A function that describes the gravitational force");
  return params;
}

NodalGravity::NodalGravity(const InputParameters & parameters)
  : NodalKernel(parameters),
    _has_mass(isParamValid("mass")),
    _has_nodal_mass_file(isParamValid("nodal_mass_file")),
    _mass(_has_mass ? getParam<Real>("mass") : 0.0),
    _alpha(getParam<Real>("alpha")),
    _gravity_value(getParam<Real>("gravity_value")),
    _function(getFunction("function"))
{
  if (!_has_nodal_mass_file && !_has_mass)
    mooseError("NodalGravity: Please provide either mass or nodal_mass_file as input.");
  else if (_has_nodal_mass_file && _has_mass)
    mooseError("NodalGravity: Please provide either mass or nodal_mass_file as input, not both.");

  if (_has_nodal_mass_file)
  {
    MooseUtils::DelimitedFileReader nodal_mass_file(getParam<FileName>("nodal_mass_file"));
    nodal_mass_file.setHeaderFlag(MooseUtils::DelimitedFileReader::HeaderFlag::OFF);
    nodal_mass_file.read();
    std::vector<std::vector<Real>> data = nodal_mass_file.getData();
    if (data.size() != 4)
      mooseError("NodalGravity: The number of columns in ",
                 getParam<FileName>("nodal_mass_file"),
                 " should be 4.");

    unsigned int node_found = 0;
    const std::set<BoundaryID> bnd_ids = BoundaryRestrictable::boundaryIDs();
    for (auto & bnd_id : bnd_ids)
    {
      const std::vector<dof_id_type> & bnd_node_set = _mesh.getNodeList(bnd_id);
      for (auto & bnd_node : bnd_node_set)
      {
        const Node & node = _mesh.nodeRef(bnd_node);
        _node_id_to_mass[bnd_node] = 0.0;

        for (unsigned int i = 0; i < data[0].size(); ++i)
        {
          if (MooseUtils::absoluteFuzzyEqual(data[0][i], node(0), 1e-6) &&
              MooseUtils::absoluteFuzzyEqual(data[1][i], node(1), 1e-6) &&
              MooseUtils::absoluteFuzzyEqual(data[2][i], node(2), 1e-6))
          {
            _node_id_to_mass[bnd_node] = data[3][i];
            node_found += 1;
            break;
          }
        }
      }
    }
    if (node_found != data[0].size())
      mooseError("NodalGravity: Out of ",
                 data[0].size(),
                 " nodal positions in ",
                 getParam<FileName>("nodal_mass_file"),
                 " only ",
                 node_found,
                 " nodes were found in the boundary.");
  }
}

Real
NodalGravity::computeQpResidual()
{
  Real mass = 0.0;
  if (_has_mass)
    mass = _mass;
  else
  {
    if (_node_id_to_mass.find(_current_node->id()) != _node_id_to_mass.end())
      mass = _node_id_to_mass[_current_node->id()];
    else
      mooseError("NodalGravity: Unable to find an entry for the current node in the "
                 "_node_id_to_mass map.");
  }
  Real factor = _gravity_value * _function.value(_t + _alpha * _dt, (*_current_node));
  return mass * -factor;
}
