//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Component2D.h"
#include "THMMesh.h"

InputParameters
Component2D::validParams()
{
  InputParameters params = GeneratedMeshComponent::validParams();
  return params;
}

Component2D::Component2D(const InputParameters & params)
  : GeneratedMeshComponent(params), _number_of_hs(0), _total_elem_number(0), _axial_offset(0.0)
{
}

void
Component2D::check() const
{
  GeneratedMeshComponent::check();

  if (isParamValid("axial_region_names"))
    checkEqualSize<std::string, Real>("axial_region_names", "length");
  else if (_n_sections > 1)
    logError("If there is more than 1 axial region, then the parameter 'axial_region_names' must "
             "be specified.");
}

bool
Component2D::hasBlock(const std::string & name) const
{
  return std::find(_names.begin(), _names.end(), name) != _names.end();
}

void
Component2D::build2DMesh()
{
  unsigned int n_axial_positions = _node_locations.size();
  std::vector<std::vector<unsigned int>> node_ids(
      n_axial_positions, std::vector<unsigned int>(_total_elem_number + 1));

  // loop over axial positions
  for (unsigned int i = 0; i < n_axial_positions; i++)
  {
    Point p(_node_locations[i], _axial_offset, 0);

    Node * nd = addNode(p);
    node_ids[i][0] = nd->id();

    // loop over all heat structures
    unsigned int l = 1;
    for (unsigned int j = 0; j < _number_of_hs; j++)
    {
      Real elem_length = _width[j] / _n_part_elems[j];
      for (unsigned int k = 0; k < _n_part_elems[j]; k++, l++)
      {
        p(1) += elem_length;
        nd = addNode(p);
        node_ids[i][l] = nd->id();
      }
      _side_heat_node_ids[_names[j]].push_back(nd->id());
    }
    _inner_heat_node_ids.push_back(node_ids[i][0]);
    _outer_heat_node_ids.push_back(node_ids[i][_total_elem_number]);
  }

  auto & boundary_info = mesh().getMesh().get_boundary_info();

  // create elements from nodes
  unsigned int i = 0;
  for (unsigned int i_section = 0; i_section < _n_sections; i_section++)
  {
    // element axial index for end of axial section
    unsigned int i_section_end = 0;
    for (unsigned int ii_section = 0; ii_section <= i_section; ++ii_section)
      i_section_end += _n_elems[ii_section];
    i_section_end -= 1;

    for (unsigned int i_local = 0; i_local < _n_elems[i_section]; i_local++)
    {
      unsigned int j = 0;
      for (unsigned int j_section = 0; j_section < _number_of_hs; j_section++)
        for (unsigned int j_local = 0; j_local < _n_part_elems[j_section]; j_local++)
        {
          Elem * elem = addElementQuad4(
              node_ids[i][j + 1], node_ids[i][j], node_ids[i + 1][j], node_ids[i + 1][j + 1]);
          elem->subdomain_id() = _subdomain_ids[j_section];

          // exterior axial boundaries (all radial sections)
          if (i == 0)
          {
            boundary_info.add_side(elem, 0, _start_bc_id[0]);
            _hs_boundary_info[_boundary_names_start[0]].push_back(
                std::tuple<dof_id_type, unsigned short int>(elem->id(), 0));
          }
          if (i == _n_elem - 1)
          {
            boundary_info.add_side(elem, 2, _end_bc_id[0]);
            _hs_boundary_info[_boundary_names_end[0]].push_back(
                std::tuple<dof_id_type, unsigned short int>(elem->id(), 2));
          }

          // exterior axial boundaries (per radial section)
          if (_names.size() > 1)
          {
            if (i == 0)
            {
              boundary_info.add_side(elem, 0, _radial_start_bc_id[j_section]);
              _hs_boundary_info[_boundary_names_radial_start[j_section]].push_back(
                  std::tuple<dof_id_type, unsigned short int>(elem->id(), 0));
            }
            if (i == _n_elem - 1)
            {
              boundary_info.add_side(elem, 2, _radial_end_bc_id[j_section]);
              _hs_boundary_info[_boundary_names_radial_end[j_section]].push_back(
                  std::tuple<dof_id_type, unsigned short int>(elem->id(), 2));
            }
          }

          // interior axial boundaries (per radial section)
          if (_n_sections > 1 && _axial_region_names.size() == _n_sections &&
              i_section != _n_sections - 1 && i == i_section_end)
          {
            const unsigned int k = i_section * _number_of_hs + j_section;
            boundary_info.add_side(elem, 2, _interior_axial_per_radial_section_bc_id[k]);
            _hs_boundary_info[_boundary_names_interior_axial_per_radial_section[k]].push_back(
                std::tuple<dof_id_type, unsigned short int>(elem->id(), 2));
          }

          // exterior radial boundaries (all axial sections)
          if (j == 0)
          {
            boundary_info.add_side(elem, 1, _inner_bc_id[0]);
            _hs_boundary_info[_boundary_names_inner[0]].push_back(
                std::tuple<dof_id_type, unsigned short int>(elem->id(), 1));
          }
          if (j == _total_elem_number - 1)
          {
            boundary_info.add_side(elem, 3, _outer_bc_id[0]);
            _hs_boundary_info[_boundary_names_outer[0]].push_back(
                std::tuple<dof_id_type, unsigned short int>(elem->id(), 3));
          }

          // exterior radial boundaries (per axial section)
          if (_n_sections > 1 && _axial_region_names.size() == _n_sections)
          {
            if (j == 0)
            {
              boundary_info.add_side(elem, 1, _axial_inner_bc_id[i_section]);
              _hs_boundary_info[_boundary_names_axial_inner[i_section]].push_back(
                  std::tuple<dof_id_type, unsigned short int>(elem->id(), 1));
            }
            if (j == _total_elem_number - 1)
            {
              boundary_info.add_side(elem, 3, _axial_outer_bc_id[i_section]);
              _hs_boundary_info[_boundary_names_axial_outer[i_section]].push_back(
                  std::tuple<dof_id_type, unsigned short int>(elem->id(), 3));
            }
          }

          // interior radial boundaries (all axial sections)
          if (_number_of_hs > 1 && _names.size() == _number_of_hs && j_section != 0)
          {
            unsigned int j_section_begin = 0;
            for (unsigned int jj_section = 0; jj_section < j_section; ++jj_section)
              j_section_begin += _n_part_elems[jj_section];

            if (j == j_section_begin)
            {
              boundary_info.add_side(elem, 1, _inner_radial_bc_id[j_section - 1]);
              _hs_boundary_info[_boundary_names_inner_radial[j_section - 1]].push_back(
                  std::tuple<dof_id_type, unsigned short int>(elem->id(), 1));
            }
          }

          j++;
        }

      i++;
    }
  }
}

void
Component2D::build2DMesh2ndOrder()
{
  // loop on flow channel nodes to create heat structure nodes
  unsigned int n_axial_positions = _node_locations.size();
  std::vector<std::vector<unsigned int>> node_ids(
      n_axial_positions, std::vector<unsigned int>(2 * _total_elem_number + 1));

  // loop over layers
  for (unsigned int i = 0; i < n_axial_positions; i++)
  {
    Point p(_node_locations[i], _axial_offset, 0);

    const Node * nd = addNode(p);
    node_ids[i][0] = nd->id();

    // loop over all heat structures
    unsigned int l = 1;
    for (unsigned int j = 0; j < _number_of_hs; j++)
    {
      Real elem_length = _width[j] / (2. * _n_part_elems[j]);
      for (unsigned int k = 0; k < 2. * _n_part_elems[j]; k++, l++)
      {
        p(1) += elem_length;
        nd = addNode(p);
        node_ids[i][l] = nd->id();
      }
      _side_heat_node_ids[_names[j]].push_back(nd->id());
    }
    _inner_heat_node_ids.push_back(node_ids[i][0]);
    _outer_heat_node_ids.push_back(node_ids[i][_total_elem_number * 2]);
  }

  auto & boundary_info = mesh().getMesh().get_boundary_info();

  // create elements from nodes
  unsigned int i = 0;
  for (unsigned int i_section = 0; i_section < _n_sections; i_section++)
    for (unsigned int i_local = 0; i_local < _n_elems[i_section]; i_local++)
    {
      unsigned int j = 0;
      for (unsigned int j_section = 0; j_section < _number_of_hs; j_section++)
        for (unsigned int j_local = 0; j_local < _n_part_elems[j_section]; j_local++)
        {
          Elem * elem = addElementQuad9(node_ids[2 * i][2 * j],
                                        node_ids[2 * i][2 * (j + 1)],
                                        node_ids[2 * (i + 1)][2 * (j + 1)],
                                        node_ids[2 * (i + 1)][2 * j],
                                        node_ids[2 * i][(2 * j) + 1],
                                        node_ids[(2 * i) + 1][2 * (j + 1)],
                                        node_ids[2 * (i + 1)][(2 * j) + 1],
                                        node_ids[(2 * i) + 1][(2 * j)],
                                        node_ids[(2 * i) + 1][(2 * j) + 1]);
          elem->subdomain_id() = _subdomain_ids[j_section];

          if (i == 0)
          {
            boundary_info.add_side(elem, 0, _start_bc_id[0]);
            _hs_boundary_info[_boundary_names_start[0]].push_back(
                std::tuple<dof_id_type, unsigned short int>(elem->id(), 0));
          }
          if (i == _n_elem - 1)
          {
            boundary_info.add_side(elem, 2, _end_bc_id[0]);
            _hs_boundary_info[_boundary_names_end[0]].push_back(
                std::tuple<dof_id_type, unsigned short int>(elem->id(), 2));
          }
          if (_names.size() > 1)
          {
            if (i == 0)
            {
              boundary_info.add_side(elem, 0, _radial_start_bc_id[j_section]);
              _hs_boundary_info[_boundary_names_radial_start[j_section]].push_back(
                  std::tuple<dof_id_type, unsigned short int>(elem->id(), 0));
            }
            if (i == _n_elem - 1)
            {
              boundary_info.add_side(elem, 2, _radial_end_bc_id[j_section]);
              _hs_boundary_info[_boundary_names_radial_end[j_section]].push_back(
                  std::tuple<dof_id_type, unsigned short int>(elem->id(), 2));
            }
          }

          if (j == 0)
          {
            boundary_info.add_side(elem, 3, _inner_bc_id[0]);
            _hs_boundary_info[_boundary_names_inner[0]].push_back(
                std::tuple<dof_id_type, unsigned short int>(elem->id(), 3));
          }
          if (j == _total_elem_number - 1)
          {
            boundary_info.add_side(elem, 1, _outer_bc_id[0]);
            _hs_boundary_info[_boundary_names_outer[0]].push_back(
                std::tuple<dof_id_type, unsigned short int>(elem->id(), 1));
          }

          if (_n_sections > 1 && _axial_region_names.size() == _n_sections)
          {
            if (j == 0)
            {
              boundary_info.add_side(elem, 1, _axial_inner_bc_id[i_section]);
              _hs_boundary_info[_boundary_names_axial_inner[i_section]].push_back(
                  std::tuple<dof_id_type, unsigned short int>(elem->id(), 1));
            }
            if (j == _total_elem_number - 1)
            {
              boundary_info.add_side(elem, 3, _axial_outer_bc_id[i_section]);
              _hs_boundary_info[_boundary_names_axial_outer[i_section]].push_back(
                  std::tuple<dof_id_type, unsigned short int>(elem->id(), 3));
            }
          }

          // interior radial boundaries
          if (_number_of_hs > 1 && _names.size() == _number_of_hs && j_section != 0)
          {
            unsigned int j_section_begin = 0;
            for (unsigned int jj_section = 0; jj_section < j_section; ++jj_section)
              j_section_begin += _n_part_elems[jj_section];

            if (j == j_section_begin)
            {
              boundary_info.add_side(elem, 1, _inner_radial_bc_id[j_section - 1]);
              _hs_boundary_info[_boundary_names_inner_radial[j_section - 1]].push_back(
                  std::tuple<dof_id_type, unsigned short int>(elem->id(), 1));
            }
          }

          j++;
        }

      i++;
    }
}

void
Component2D::buildMesh()
{
  if (_n_part_elems.size() != _number_of_hs || _width.size() != _number_of_hs)
    return;

  // Assign subdomain to each transverse region
  for (unsigned int i = 0; i < _number_of_hs; i++)
  {
    // The coordinate system for MOOSE is always XYZ, even for axisymmetric
    // components, since we do the RZ integration ourselves until we can set
    // arbitrary number of axis symmetries in MOOSE.
    setSubdomainInfo(mesh().getNextSubdomainId(), genName(_name, _names[i]), Moose::COORD_XYZ);
  }

  // Create boundary IDs and associated boundary names
  _inner_bc_id.push_back(mesh().getNextBoundaryId());
  _outer_bc_id.push_back(mesh().getNextBoundaryId());
  _boundary_names_inner.push_back(genName(name(), "inner"));
  _boundary_names_outer.push_back(genName(name(), "outer"));
  if (_n_sections > 1 && _axial_region_names.size() == _n_sections)
    for (unsigned int i = 0; i < _n_sections; i++)
    {
      _axial_inner_bc_id.push_back(mesh().getNextBoundaryId());
      _axial_outer_bc_id.push_back(mesh().getNextBoundaryId());
      _boundary_names_axial_inner.push_back(genName(name(), _axial_region_names[i], "inner"));
      _boundary_names_axial_outer.push_back(genName(name(), _axial_region_names[i], "outer"));
    }

  // exterior axial boundaries
  _start_bc_id.push_back(mesh().getNextBoundaryId());
  _end_bc_id.push_back(mesh().getNextBoundaryId());
  _boundary_names_start.push_back(genName(name(), "start"));
  _boundary_names_end.push_back(genName(name(), "end"));
  if (_names.size() > 1)
    for (unsigned int i = 0; i < _names.size(); i++)
    {
      _radial_start_bc_id.push_back(mesh().getNextBoundaryId());
      _radial_end_bc_id.push_back(mesh().getNextBoundaryId());
      _boundary_names_radial_start.push_back(genName(name(), _names[i], "start"));
      _boundary_names_radial_end.push_back(genName(name(), _names[i], "end"));
      if (i != _names.size() - 1)
      {
        _inner_radial_bc_id.push_back(mesh().getNextBoundaryId());
        _boundary_names_inner_radial.push_back(genName(name(), _names[i], _names[i + 1]));
      }
    }

  // interior axial boundaries
  if (_n_sections > 1 && _axial_region_names.size() == _n_sections)
    for (unsigned int i = 0; i < _n_sections - 1; i++)
      for (unsigned int j = 0; j < _names.size(); j++)
      {
        _interior_axial_per_radial_section_bc_id.push_back(mesh().getNextBoundaryId());
        _boundary_names_interior_axial_per_radial_section.push_back(
            genName(name(), _names[j], _axial_region_names[i] + ":" + _axial_region_names[i + 1]));
      }

  // Build the mesh
  if (usingSecondOrderMesh())
    build2DMesh2ndOrder();
  else
    build2DMesh();

  // Set boundary names
  auto & binfo = mesh().getMesh().get_boundary_info();
  binfo.sideset_name(_inner_bc_id[0]) = _boundary_names_inner[0];
  binfo.sideset_name(_outer_bc_id[0]) = _boundary_names_outer[0];
  if (_n_sections > 1 && _axial_region_names.size() == _n_sections)
    for (unsigned int i = 0; i < _n_sections; i++)
    {
      binfo.sideset_name(_axial_inner_bc_id[i]) = _boundary_names_axial_inner[i];
      binfo.sideset_name(_axial_outer_bc_id[i]) = _boundary_names_axial_outer[i];
    }
  binfo.sideset_name(_start_bc_id[0]) = _boundary_names_start[0];
  binfo.sideset_name(_end_bc_id[0]) = _boundary_names_end[0];
  if (_names.size() > 1)
    for (unsigned int i = 0; i < _names.size(); i++)
    {
      binfo.sideset_name(_radial_start_bc_id[i]) = _boundary_names_radial_start[i];
      binfo.sideset_name(_radial_end_bc_id[i]) = _boundary_names_radial_end[i];
      if (i != _names.size() - 1)
        binfo.sideset_name(_inner_radial_bc_id[i]) = _boundary_names_inner_radial[i];
    }
  for (unsigned int k = 0; k < _interior_axial_per_radial_section_bc_id.size(); k++)
    binfo.sideset_name(_interior_axial_per_radial_section_bc_id[k]) =
        _boundary_names_interior_axial_per_radial_section[k];
}

bool
Component2D::isBoundaryInVector(const BoundaryName & boundary_name,
                                const std::vector<BoundaryName> & boundary_name_vector) const
{
  return std::find(boundary_name_vector.begin(), boundary_name_vector.end(), boundary_name) !=
         boundary_name_vector.end();
}

bool
Component2D::hasBoundary(const BoundaryName & boundary_name) const
{
  return isBoundaryInVector(boundary_name, _boundary_names_inner) ||
         isBoundaryInVector(boundary_name, _boundary_names_axial_inner) ||
         isBoundaryInVector(boundary_name, _boundary_names_outer) ||
         isBoundaryInVector(boundary_name, _boundary_names_axial_outer) ||
         isBoundaryInVector(boundary_name, _boundary_names_start) ||
         isBoundaryInVector(boundary_name, _boundary_names_radial_start) ||
         isBoundaryInVector(boundary_name, _boundary_names_end) ||
         isBoundaryInVector(boundary_name, _boundary_names_radial_end) ||
         isBoundaryInVector(boundary_name, _boundary_names_interior_axial_per_radial_section) ||
         isBoundaryInVector(boundary_name, _boundary_names_inner_radial);
}

const std::vector<std::tuple<dof_id_type, unsigned short int>> &
Component2D::getBoundaryInfo(const BoundaryName & boundary_name) const
{
  if (_hs_boundary_info.find(boundary_name) != _hs_boundary_info.end())
    return _hs_boundary_info.at(boundary_name);
  else
    mooseError("The heat structure boundary '",
               boundary_name,
               "' does not exist for the heat structure '",
               name(),
               "'.");
}
