//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComboCutUserObject.h"

registerMooseObject("XFEMApp", ComboCutUserObject);

InputParameters
ComboCutUserObject::validParams()
{
  InputParameters params = GeometricCutUserObject::validParams();
  params.addClassDescription("Combine multiple geometric cut userobjects.");
  params.addRequiredParam<std::vector<UserObjectName>>("cuts",
                                                       "Vector of cut userobjects to combine");
  params.addRequiredParam<std::vector<GeometricCutSubdomainID>>(
      "keys", "Possible combinations of the geometric cut subdomain IDs");
  params.addRequiredParam<std::vector<GeometricCutSubdomainID>>(
      "vals", "Resulting combined geometric cut subdomain ID for each of the combination");

  return params;
}

ComboCutUserObject::ComboCutUserObject(const InputParameters & parameters)
  : GeometricCutUserObject(parameters),
    _cut_names(getParam<std::vector<UserObjectName>>("cuts")),
    _num_cuts(_cut_names.size()),
    _cuts(_num_cuts),
    _keys(getParam<std::vector<GeometricCutSubdomainID>>("keys")),
    _vals(getParam<std::vector<GeometricCutSubdomainID>>("vals"))
{
  for (unsigned int i = 0; i < _num_cuts; i++)
    _cuts[i] = &getUserObjectByName<GeometricCutUserObject>(_cut_names[i]);

  buildMap();
}

void
ComboCutUserObject::buildMap()
{
  if (_keys.size() % _num_cuts != 0)
    mooseError("Expect multiples of ", _num_cuts, " keys, but got ", _keys.size(), " keys.");

  unsigned int num_combos = _keys.size() / _num_cuts;

  if (_vals.size() != num_combos)
    mooseError("Expect one value for each of the combination, got ",
               num_combos,
               " combinations, but got ",
               _vals.size(),
               " values.");

  for (unsigned int i = 0; i < num_combos; i++)
  {
    std::vector<GeometricCutSubdomainID> combo_key;
    for (unsigned int j = 0; j < _num_cuts; j++)
      combo_key.push_back(_keys[i * _num_cuts + j]);
    _combo_ids.emplace(combo_key, _vals[i]);
  }
}

bool
ComboCutUserObject::cutElementByGeometry(const Elem * elem,
                                         std::vector<Xfem::CutEdge> & cut_edges,
                                         std::vector<Xfem::CutNode> & cut_nodes,
                                         Real time) const
{
  for (auto cut : _cuts)
    if (cut->cutElementByGeometry(elem, cut_edges, cut_nodes, time))
      return true;
  return false;
}

bool
ComboCutUserObject::cutElementByGeometry(const Elem * elem,
                                         std::vector<Xfem::CutFace> & cut_faces,
                                         Real time) const
{
  for (auto cut : _cuts)
    if (cut->cutElementByGeometry(elem, cut_faces, time))
      return true;
  return false;
}

bool
ComboCutUserObject::cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                          std::vector<Xfem::CutEdge> & cut_edges,
                                          Real time) const
{
  for (auto cut : _cuts)
    if (cut->cutFragmentByGeometry(frag_edges, cut_edges, time))
      return true;
  return false;
}

bool
ComboCutUserObject::cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                          std::vector<Xfem::CutFace> & cut_faces,
                                          Real time) const
{
  for (auto cut : _cuts)
    if (cut->cutFragmentByGeometry(frag_faces, cut_faces, time))
      return true;
  return false;
}

GeometricCutSubdomainID
ComboCutUserObject::getGeometricCutSubdomainID(const Node * node) const
{
  std::vector<GeometricCutSubdomainID> combo_key;
  for (auto cut : _cuts)
    combo_key.push_back(cut->getGeometricCutSubdomainID(node));
  return _combo_ids.at(combo_key);
}
