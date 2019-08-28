#include "FlowChannelAlignment.h"
#include "FlowChannelBase.h"
#include "KDTree.h"
#include "libmesh/fe_interface.h"

FlowChannelAlignment::FlowChannelAlignment(const FlowChannelBase & flow_channel,
                                           const BoundaryName & master_bnd_name,
                                           const BoundaryName & slave_bnd_name)
  : _flow_channel(flow_channel),
    _mesh(_flow_channel.mesh()),
    _master_bnd_id(_mesh.getBoundaryID(master_bnd_name)),
    _slave_bnd_id(_mesh.getBoundaryID(slave_bnd_name))
{
}

void
FlowChannelAlignment::build()
{
  // element ids corresponding to the centroids in `master_points`
  std::vector<dof_id_type> master_elem_ids;
  // local side number corresponding to the element id in `master_elem_ids`
  std::vector<dof_id_type> master_elem_sides;
  // element ids corresponding to the centroids in `slave_points`
  std::vector<dof_id_type> slave_elem_ids;

  ConstBndElemRange & range = *_mesh.getBoundaryElementRange();
  for (const auto & belem : range)
  {
    const Elem * elem = belem->_elem;
    BoundaryID boundary_id = belem->_bnd_id;

    if (boundary_id == _master_bnd_id)
    {
      // 2D elements
      master_elem_ids.push_back(elem->id());
      master_elem_sides.push_back(belem->_side);
      _master_points.push_back(elem->centroid());
      _nearest_elem_side.insert(std::pair<dof_id_type, unsigned int>(elem->id(), belem->_side));
    }
    else if (boundary_id == _slave_bnd_id)
    {
      if (std::find(slave_elem_ids.begin(), slave_elem_ids.end(), elem->id()) ==
          slave_elem_ids.end())
      {
        // 1D elements
        slave_elem_ids.push_back(elem->id());
        _slave_points.push_back(elem->centroid());
      }
    }
  }

  if (_master_points.size() > 0 && _slave_points.size() > 0)
  {
    // find the master elements that are nearest to the slave elements
    KDTree kd_tree(_master_points, _mesh.getMaxLeafSize());
    for (std::size_t i = 0; i < _slave_points.size(); i++)
    {
      unsigned int patch_size = 1;
      std::vector<std::size_t> return_index(patch_size);
      kd_tree.neighborSearch(_slave_points[i], patch_size, return_index);

      _nearest_elem_ids.insert(
          std::pair<dof_id_type, dof_id_type>(slave_elem_ids[i], master_elem_ids[return_index[0]]));
      _nearest_elem_ids.insert(
          std::pair<dof_id_type, dof_id_type>(master_elem_ids[return_index[0]], slave_elem_ids[i]));
    }
  }
}

bool
FlowChannelAlignment::check() const
{
  if (_master_points.size() > 0 && _slave_points.size() > 0)
  {
    // Go over all elements in the flow channel. Take the center of each element and project it onto
    // the side of the 2D component. Then check that the projected location matches the location of
    // the original center of the flow channel element
    const std::vector<unsigned int> & fch_elem_ids = _flow_channel.getElementIDs();
    for (const auto & elem_id : fch_elem_ids)
    {
      const Elem * elem = _mesh.elemPtr(elem_id);
      Point center_pt = elem->centroid();

      const dof_id_type & hs_elem_id = _nearest_elem_ids.at(elem_id);
      const unsigned int & hs_elem_side = _nearest_elem_side.at(hs_elem_id);
      const Elem * neighbor = _mesh.elemPtr(hs_elem_id);
      const Elem * neighbor_side_elem = neighbor->build_side_ptr(hs_elem_side).release();
      unsigned int neighbor_dim = neighbor_side_elem->dim();
      Point ref_pt =
          FEInterface::inverse_map(neighbor_dim, FEType(), neighbor_side_elem, center_pt);
      Point hs_pt = FEInterface::map(neighbor_dim, FEType(), neighbor_side_elem, ref_pt);
      delete neighbor_side_elem;

      if (!center_pt.absolute_fuzzy_equals(hs_pt))
        return false;
    }
  }

  return true;
}
