//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvancedConcentricCircleGenerator.h"

// C++ includes
#include <cmath>

registerMooseObject("ReactorApp", AdvancedConcentricCircleGenerator);

InputParameters
AdvancedConcentricCircleGenerator::validParams()
{
  InputParameters params = ConcentricCircleGeneratorBase::validParams();

  params.makeParamRequired<std::vector<Real>>("ring_radii");
  params.makeParamRequired<std::vector<unsigned int>>("ring_intervals");

  params.addRangeCheckedParam<unsigned int>(
      "num_sectors",
      "num_sectors>2",
      "Number of azimuthal sectors of the circular mesh to be generated.");
  params.addRangeCheckedParam<std::vector<Real>>(
      "customized_azimuthal_angles",
      "customized_azimuthal_angles>=0&customized_azimuthal_angles<360",
      "List of the user-specified azimuthal angles of the nodes.");

  params.addParamNamesToGroup("num_sectors customized_azimuthal_angles", "Azimuthal Mesh Density");

  params.addClassDescription("This AdvancedConcentricCircleGenerator object is designed to mesh a "
                             "concentric circular geometry.");

  return params;
}

AdvancedConcentricCircleGenerator::AdvancedConcentricCircleGenerator(
    const InputParameters & parameters)
  : ConcentricCircleGeneratorBase(parameters),
    _azimuthal_angles(isParamValid("customized_azimuthal_angles")
                          ? getParam<std::vector<Real>>("customized_azimuthal_angles")
                          : std::vector<Real>()),
    _num_sectors(isParamValid("num_sectors") ? getParam<unsigned int>("num_sectors")
                                             : _azimuthal_angles.size())
{
  if (_num_sectors == 0)
    paramError(
        "num_sectors",
        "this parameter must be specified if 'customized_azimuthal_angles' is not provided.");
  if (_azimuthal_angles.empty())
  {
    for (unsigned int i = 0; i < _num_sectors; i++)
    {
      _azimuthal_angles.push_back((Real)i * 360.0 / (Real)_num_sectors);
      _virtual_nums_sectors.push_back((Real)_num_sectors);
    }
  }
  else
  {
    if (_num_sectors != _azimuthal_angles.size())
      paramError("num_sectors",
                 "this parameter must be equal to the size of 'customized_azimuthal_angles' if "
                 "both parameters are provided.");
    for (unsigned int i = 0; i < _azimuthal_angles.size(); i++)
    {
      const Real azi_angle_interval = i == _azimuthal_angles.size() - 1
                                          ? 360.0 + _azimuthal_angles[0] - _azimuthal_angles[i]
                                          : _azimuthal_angles[i + 1] - _azimuthal_angles[i];
      if (azi_angle_interval <= 0.0)
        paramError("customized_azimuthal_angles",
                   "the azimuthal angles provided must be strictly increasing.");
      else if (azi_angle_interval >= 120.0)
        paramError("customized_azimuthal_angles",
                   "please make sure the circle azimuthal discretization angles are less than "
                   "120.0 to avert awkward polygonization.");
      _virtual_nums_sectors.push_back(360.0 / azi_angle_interval);
    }
  }
  // Customized interface boundary id/name related error messages
  if (!_create_inward_interface_boundaries && _inward_interface_boundary_names.size() > 0)
    paramError("create_inward_interface_boundaries",
               "If set false, 'inward_interface_boundary_names' "
               "should not be provided as they are not used.");
  if (!_create_outward_interface_boundaries && _outward_interface_boundary_names.size() > 0)
    paramError("create_outward_interface_boundaries",
               "If set false, 'outward_interface_boundary_names' "
               "should not be provided as they are not used.");
  if (!_create_outward_interface_boundaries && !_create_inward_interface_boundaries &&
      _interface_boundary_id_shift != 0)
    paramError("interface_boundary_id_shift",
               "this parameter should not be set if no interface boundaries are created.");
  if (_inward_interface_boundary_names.size() > 0 &&
      _inward_interface_boundary_names.size() != _ring_radii.size() - 1)
    paramError("inward_interface_boundary_names",
               "If provided, the length of this parameter must be identical to the total number of "
               "interfaces.");
  if (_outward_interface_boundary_names.size() > 0 &&
      _outward_interface_boundary_names.size() != _ring_radii.size() - 1)
    paramError("outward_interface_boundary_names",
               "If provided, the length of this parameter must be identical to the total number of "
               "interfaces.");

  // Rings related error messages
  if (_ring_radii.size() != _ring_intervals.size())
    paramError("ring_radii", "This parameter and ring_intervals must have the same length.");
  if (_ring_radii.size() != _ring_radial_biases.size())
    paramError("ring_radii", "This parameter and ring_radial_biases must have the same length.");
  for (unsigned int i = 1; i < _ring_intervals.size(); i++)
    if (_ring_radii[i] <= _ring_radii[i - 1])
      paramError("ring_radii", "This parameter must be strictly ascending.");
  if (_ring_radii.size() != _ring_inner_boundary_layer_params.widths.size() ||
      _ring_radii.size() != _ring_inner_boundary_layer_params.intervals.size() ||
      _ring_radii.size() != _ring_inner_boundary_layer_params.biases.size() ||
      _ring_radii.size() != _ring_outer_boundary_layer_params.widths.size() ||
      _ring_radii.size() != _ring_outer_boundary_layer_params.intervals.size() ||
      _ring_radii.size() != _ring_outer_boundary_layer_params.biases.size())
    paramError("ring_radii",
               "The inner and outer ring boundary layer parameters must have the same sizes as "
               "ring_radii.");

  const unsigned int num_innermost_ring_layers =
      _ring_inner_boundary_layer_params.intervals.front() + _ring_intervals.front() +
      _ring_outer_boundary_layer_params.intervals.front();
  if (!_ring_block_ids.empty() &&
      _ring_block_ids.size() !=
          (_ring_intervals.size() + (unsigned int)(num_innermost_ring_layers != 1)))
    paramError("ring_block_ids",
               "This parameter must have the appropriate size if it is provided. The size should "
               "be the same as the size of 'ring_intervals' if the innermost ring interval "
               "(including boundary layers) is unity; otherwise the size should be greater than "
               "the size of 'ring_intervals' by one. If 'quad_center_elements' is true, it is "
               "optional to only provide this parameter with the same size as 'ring_intervals'");
  if (!_ring_block_names.empty() &&
      _ring_block_names.size() !=
          (_ring_intervals.size() + (unsigned int)(num_innermost_ring_layers != 1)))
    paramError("ring_block_names",
               "This parameter must have the appropriate size if it is set. The size should be the "
               "same as the size of 'ring_intervals' if the innermost ring interval (including "
               "boundary layers) is unity; otherwise the size should be greater than the size of "
               "'ring_intervals' by one. If 'quad_center_elements' is true, it is optional to only "
               "provide this parameter with the same size as 'ring_intervals'");
  for (unsigned int i = 0; i < _ring_radii.size(); i++)
  {
    const Real layer_width = _ring_radii[i] - (i == 0 ? 0.0 : _ring_radii[i - 1]);
    _ring_inner_boundary_layer_params.fractions.push_back(
        _ring_inner_boundary_layer_params.widths[i] / layer_width);
    _ring_outer_boundary_layer_params.fractions.push_back(
        _ring_outer_boundary_layer_params.widths[i] / layer_width);
  }
  for (unsigned int i = 0; i < _ring_inner_boundary_layer_params.fractions.size(); i++)
    if (MooseUtils::absoluteFuzzyEqual(_ring_inner_boundary_layer_params.fractions[i], 0.0) &&
        _ring_inner_boundary_layer_params.intervals[i] > 0)
      paramError("ring_inner_boundary_layer_intervals",
                 "Ring inner boundary layer must have zero interval if its thickness is zero.");
    else if (MooseUtils::absoluteFuzzyGreaterThan(_ring_inner_boundary_layer_params.fractions[i],
                                                  0.0) &&
             _ring_inner_boundary_layer_params.intervals[i] == 0)
      paramError(
          "ring_inner_boundary_layer_intervals",
          "Ring inner boundary layer must have non-zero interval if its thickness is not zero.");
  for (unsigned int i = 0; i < _ring_outer_boundary_layer_params.fractions.size(); i++)
  {
    if (MooseUtils::absoluteFuzzyEqual(_ring_outer_boundary_layer_params.fractions[i], 0.0) &&
        _ring_outer_boundary_layer_params.intervals[i] > 0)
      paramError("ring_outer_boundary_layer_intervals",
                 "Ring outer boundary layer must have zero interval if its thickness is zero.");
    else if (MooseUtils::absoluteFuzzyGreaterThan(_ring_outer_boundary_layer_params.fractions[i],
                                                  0.0) &&
             _ring_outer_boundary_layer_params.intervals[i] == 0)
      paramError(
          "ring_outer_boundary_layer_intervals",
          "Ring outer boundary layer must have non-zero interval if its thickness is not zero.");
    if (_ring_inner_boundary_layer_params.fractions[i] +
            _ring_outer_boundary_layer_params.fractions[i] >=
        1.0)
      paramError("ring_inner_boundary_layer_widths",
                 "Summation of ring_inner_boundary_layer_widths and "
                 "ring_outer_boundary_layer_widths cannot exceeds the ring layer width.");
  }

  for (unsigned int i = 0; i < _ring_radii.size(); i++)
  {
    const Real layer_width = _ring_radii[i] - (i == 0 ? 0.0 : _ring_radii[i - 1]);
    _ring_inner_boundary_layer_params.fractions.push_back(
        _ring_inner_boundary_layer_params.widths[i] / layer_width);
    _ring_outer_boundary_layer_params.fractions.push_back(
        _ring_outer_boundary_layer_params.widths[i] / layer_width);
  }
}

std::unique_ptr<MeshBase>
AdvancedConcentricCircleGenerator::generate()
{
  std::vector<Real> ring_radii_corr;
  const Real corr_factor = _preserve_volumes ? radiusCorrectionFactor(_azimuthal_angles) : 1.0;

  for (const auto & ring_radius : _ring_radii)
    ring_radii_corr.push_back(ring_radius * corr_factor);

  const multiBdryLayerParams empty_params = {
      std::vector<Real>(), std::vector<Real>(), std::vector<unsigned int>(), std::vector<Real>()};
  const singleBdryLayerParams empty_param = {0.0, 0.0, 0, 1.0};

  // A dummy pitch number is needed for callind buildSlice()
  // Any value larger than twice of the largest ring radius will work
  const Real dummy_pitch = _ring_radii.back() * 3.0;
  const unsigned int num_sectors_per_side = 1;
  const unsigned int background_intervals = 0;
  const Real background_radial_bias = 1.0;

  dof_id_type node_id_background_meta;

  auto mesh = buildSlice(ring_radii_corr,
                         _ring_intervals,
                         _ring_radial_biases,
                         _ring_inner_boundary_layer_params,
                         _ring_outer_boundary_layer_params,
                         std::vector<Real>(),
                         std::vector<unsigned int>(),
                         std::vector<Real>(),
                         empty_params,
                         empty_params,
                         dummy_pitch,
                         num_sectors_per_side,
                         background_intervals,
                         background_radial_bias,
                         empty_param,
                         empty_param,
                         node_id_background_meta,
                         _virtual_nums_sectors[0],
                         /*side_index*/ 1,
                         std::vector<Real>(),
                         _block_id_shift,
                         /* quad_center_elements */ false,
                         /* center_quad_factor */ 0.0,
                         _create_inward_interface_boundaries,
                         _create_outward_interface_boundaries,
                         _interface_boundary_id_shift);
  MeshTools::Modification::rotate(*mesh, -_azimuthal_angles[0], 0, 0);

  for (unsigned int i = 1; i < _num_sectors; i++)
  {
    auto mesh_tmp = buildSlice(ring_radii_corr,
                               _ring_intervals,
                               _ring_radial_biases,
                               _ring_inner_boundary_layer_params,
                               _ring_outer_boundary_layer_params,
                               std::vector<Real>(),
                               std::vector<unsigned int>(),
                               std::vector<Real>(),
                               empty_params,
                               empty_params,
                               dummy_pitch,
                               num_sectors_per_side,
                               background_intervals,
                               background_radial_bias,
                               empty_param,
                               empty_param,
                               node_id_background_meta,
                               _virtual_nums_sectors[i],
                               /*side_index*/ i + 1,
                               std::vector<Real>(),
                               _block_id_shift,
                               /* quad_center_elements */ false,
                               /* center_quad_factor */ 0.0,
                               _create_inward_interface_boundaries,
                               _create_outward_interface_boundaries,
                               _interface_boundary_id_shift);

    ReplicatedMesh other_mesh(*mesh_tmp);
    MeshTools::Modification::rotate(other_mesh, -_azimuthal_angles[i], 0, 0);
    mesh->prepare_for_use();
    other_mesh.prepare_for_use();
    mesh->stitch_meshes(other_mesh, SLICE_BEGIN, SLICE_END, TOLERANCE, true);
    other_mesh.clear();
  }

  if (!_generate_side_specific_boundaries)
    for (unsigned int i = 0; i < _num_sectors; i++)
      mesh->get_boundary_info().remove_id(i + 1 + OUTER_SIDESET_ID_ALT);

  // An extra step to stich the first and last slices together
  mesh->stitch_surfaces(SLICE_BEGIN, SLICE_END, TOLERANCE, true);

  mesh->prepare_for_use();

  // Set up customized Block Names and/or IDs
  unsigned int block_it = 0;
  unsigned int ring_block_num = 0;
  std::vector<subdomain_id_type> block_ids_old;
  std::vector<subdomain_id_type> block_ids_new;
  std::vector<SubdomainName> block_names;
  if (_ring_intervals.front() == 1)
    ring_block_num = _ring_intervals.size();
  else
  {
    ring_block_num = _ring_intervals.size() + 1;
    block_ids_old.push_back(_block_id_shift + 1);
    block_ids_new.push_back(_ring_block_ids.empty() ? block_ids_old.back()
                                                    : _ring_block_ids.front());
    block_names.push_back(_ring_block_names.empty()
                              ? (SubdomainName)std::to_string(block_ids_new.back())
                              : _ring_block_names.front());
    block_it++;
  }
  for (unsigned int i = ring_block_num - _ring_intervals.size(); i < ring_block_num; i++)
  {
    block_ids_old.push_back(_block_id_shift + 1 + i);
    block_ids_new.push_back(_ring_block_ids.empty() ? block_ids_old.back() : _ring_block_ids[i]);
    block_names.push_back(_ring_block_names.empty()
                              ? (SubdomainName)std::to_string(block_ids_new.back())
                              : _ring_block_names[i]);
    block_it++;
  }
  for (auto it = block_names.begin(); it != block_names.end() - 1; it++)
  {
    auto it_tmp = std::find(block_names.begin(), it + 1, *(it + 1));
    if (it_tmp != it + 1 && block_ids_new[std::distance(block_names.begin(), it + 1)] !=
                                block_ids_new[std::distance(block_names.begin(), it_tmp)])
      mooseError("In AdvancedConcentricCircleGenerator ",
                 _name,
                 ": blocks with different ids cannot have the same block name.");
  }
  for (const auto & elem : mesh->element_ptr_range())
    for (unsigned i = 0; i < block_ids_old.size(); ++i)
      if (elem->subdomain_id() == block_ids_old[i])
      {
        elem->subdomain_id() = block_ids_new[i];
        break;
      }
  for (unsigned i = 0; i < block_ids_new.size(); ++i)
    mesh->subdomain_name(block_ids_new[i]) = block_names[i];

  // Customized boundary ids and names
  if (_external_boundary_id > 0)
    MooseMesh::changeBoundaryId(*mesh, OUTER_SIDESET_ID, _external_boundary_id, true);
  else
    MooseMesh::changeBoundaryId(*mesh, OUTER_SIDESET_ID, 0, true);
  if (!_external_boundary_name.empty())
  {
    mesh->get_boundary_info().sideset_name(_external_boundary_id > 0 ? _external_boundary_id : 0) =
        _external_boundary_name;
    mesh->get_boundary_info().nodeset_name(_external_boundary_id > 0 ? _external_boundary_id : 0) =
        _external_boundary_name;
  }

  assignInterfaceBoundaryNames(*mesh);

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
