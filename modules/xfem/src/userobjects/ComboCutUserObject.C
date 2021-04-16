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
  params.addRequiredParam<std::vector<UserObjectName>>(
      "geometric_cut_userobjects", "Vector of geometric cut userobjects to combine");
  params.addRequiredParam<std::vector<CutSubdomainID>>(
      "cut_subdomain_combinations",
      "Possible combinations of the cut subdomain IDs. The sequence of each combination should "
      "follow the sequence provided in the geometric_cut_userobjects parameter.");
  params.addRequiredParam<std::vector<CutSubdomainID>>(
      "cut_subdomains", "Resulting combined cut subdomain IDs for each of the combination.");

  return params;
}

ComboCutUserObject::ComboCutUserObject(const InputParameters & parameters)
  : GeometricCutUserObject(parameters),
    _cut_names(getParam<std::vector<UserObjectName>>("geometric_cut_userobjects")),
    _num_cuts(_cut_names.size()),
    _cuts(_num_cuts),
    _keys(getParam<std::vector<CutSubdomainID>>("cut_subdomain_combinations")),
    _vals(getParam<std::vector<CutSubdomainID>>("cut_subdomains"))
{
  for (unsigned int i = 0; i < _num_cuts; i++)
    _cuts[i] = &getUserObjectByName<GeometricCutUserObject>(_cut_names[i]);

  buildMap();
}

void
ComboCutUserObject::buildMap()
{
  if (_keys.size() % _num_cuts != 0)
    mooseError("Expected multiples of ", _num_cuts, " keys, but got ", _keys.size(), " keys.");

  unsigned int num_combos = _keys.size() / _num_cuts;

  if (_vals.size() != num_combos)
    mooseError("Expected one value for each of the combination, got ",
               num_combos,
               " combinations, but got ",
               _vals.size(),
               " values.");

  for (unsigned int i = 0; i < num_combos; i++)
  {
    std::vector<CutSubdomainID> combo_key;
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

CutSubdomainID
ComboCutUserObject::getCutSubdomainID(const Node * node) const
{
  std::vector<CutSubdomainID> combo_key;
  for (auto cut : _cuts)
    combo_key.push_back(cut->getCutSubdomainID(node));

  try
  {
    CutSubdomainID combo_id = _combo_ids.at(combo_key);
    return combo_id;
  }
  catch (std::out_of_range &)
  {
    throw MooseException(name() + ": Unknown cut subdomain combination.");
  }
}
