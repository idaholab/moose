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
  params.addRequiredParam<std::vector<std::vector<CutSubdomainID>>>(
      "cut_subdomain_combinations",
      "Possible combinations of the cut subdomain IDs. The sequence of each combination should "
      "follow the sequence provided in the geometric_cut_userobjects parameter. Use semicolons to "
      "separate different combinations.");
  params.addRequiredParam<std::vector<CutSubdomainID>>(
      "cut_subdomains", "Resulting combined cut subdomain IDs for each of the combination.");

  return params;
}

ComboCutUserObject::ComboCutUserObject(const InputParameters & parameters)
  : GeometricCutUserObject(parameters),
    _cut_names(getParam<std::vector<UserObjectName>>("geometric_cut_userobjects")),
    _num_cuts(_cut_names.size()),
    _cuts(_num_cuts),
    _keys(getParam<std::vector<std::vector<CutSubdomainID>>>("cut_subdomain_combinations")),
    _vals(getParam<std::vector<CutSubdomainID>>("cut_subdomains"))
{
  for (unsigned int i = 0; i < _num_cuts; i++)
    _cuts[i] = &getUserObjectByName<GeometricCutUserObject>(_cut_names[i]);

  buildMap();
}

void
ComboCutUserObject::buildMap()
{
  for (const auto & combo : _keys)
    if (combo.size() != _num_cuts)
      mooseError("Expected multiples of ", _num_cuts, " keys, but got ", combo.size(), " keys.");

  unsigned int num_combos = _keys.size();

  if (_vals.size() != num_combos)
    mooseError("Expected one value for each of the combination, got ",
               num_combos,
               " combinations, but got ",
               _vals.size(),
               " values.");

  for (unsigned int i = 0; i < num_combos; i++)
    _combo_ids.emplace(_keys[i], _vals[i]);
}

bool
ComboCutUserObject::cutElementByGeometry(const Elem * elem,
                                         std::vector<Xfem::CutEdge> & cut_edges,
                                         std::vector<Xfem::CutNode> & cut_nodes) const
{
  unsigned int i_want_to_cut = 0;
  for (auto cut : _cuts)
    if (cut->cutElementByGeometry(elem, cut_edges, cut_nodes))
      i_want_to_cut++;

  // TODO: Currently we error out if more than one cut userobject wants to do the cutting. We need
  // to remove this limitation once we add the ability to handle multiple cuts.
  if (i_want_to_cut > 1)
    mooseError("More than one GeometricCutUserObject want to cut the same element.");

  return i_want_to_cut > 0;
}

bool
ComboCutUserObject::cutElementByGeometry(const Elem * elem,
                                         std::vector<Xfem::CutFace> & cut_faces) const
{
  unsigned int i_want_to_cut = 0;
  for (auto cut : _cuts)
    if (cut->cutElementByGeometry(elem, cut_faces))
      i_want_to_cut++;

  // TODO: Currently we error out if more than one cut userobject wants to do the cutting. We need
  // to remove this limitation once we add the ability to handle multiple cuts.
  if (i_want_to_cut > 1)
    mooseError("More than one GeometricCutUserObject want to cut the same element.");

  return i_want_to_cut > 0;
}

bool
ComboCutUserObject::cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                          std::vector<Xfem::CutEdge> & cut_edges) const
{
  unsigned int i_want_to_cut = 0;
  for (auto cut : _cuts)
    if (cut->cutFragmentByGeometry(frag_edges, cut_edges))
      i_want_to_cut++;

  // TODO: Currently we error out if more than one cut userobject wants to do the cutting. We need
  // to remove this limitation once we add the ability to handle multiple cuts.
  if (i_want_to_cut > 1)
    mooseError("More than one GeometricCutUserObject want to cut the same fragment.");

  return i_want_to_cut > 0;
}

bool
ComboCutUserObject::cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                          std::vector<Xfem::CutFace> & cut_faces) const
{
  unsigned int i_want_to_cut = 0;
  for (auto cut : _cuts)
    if (cut->cutFragmentByGeometry(frag_faces, cut_faces))
      i_want_to_cut++;

  // TODO: Currently we error out if more than one cut userobject wants to do the cutting. We need
  // to remove this limitation once we add the ability to handle multiple cuts.
  if (i_want_to_cut > 1)
    mooseError("More than one GeometricCutUserObject want to cut the same fragment.");

  return i_want_to_cut > 0;
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
