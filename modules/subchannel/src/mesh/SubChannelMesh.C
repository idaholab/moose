#include "SubChannelMesh.h"

#include <cmath>

#include "libmesh/edge_edge2.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("SubChannelApp", SubChannelMesh);

InputParameters
SubChannelMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
  params.addParam<unsigned int>("nx", 1, "Number of channels in the x direction");
  params.addParam<unsigned int>("ny", 1, "Number of channels in the x direction");
  params.addParam<Real>("max_dz", 0.1, "The maximum element height in meters");
  params.addParam<Real>("pitch", 0.1, "pitch in meters");
  params.addParam<Real>("rod_diameter", 1.0, "Rod Diameter in meters");
  params.addParam<Real>("gap", 1.0, "half gap between assemblies in meters");
  params.addParam<Real>("heated_length", 3.658, " heated length in meters");
  return params;
}

SubChannelMesh::SubChannelMesh(const InputParameters & params)
  : MooseMesh(params),
    nx_(getParam<unsigned int>("nx")),
    ny_(getParam<unsigned int>("ny")),
    n_channels_(nx_ * ny_),
    n_gaps_((nx_ - 1) * ny_ + (ny_ - 1) * nx_),
    pitch_(getParam<Real>("pitch")),
    rod_diameter_(getParam<Real>("rod_diameter")),
    gap_(getParam<Real>("gap")),
    heated_length_(getParam<Real>("heated_length")),
    max_dz_(getParam<Real>("max_dz"))
{
  // TODO: parameterize
  std::vector<Real> spacer_z({0,
                              0.229,
                              0.457,
                              0.686,
                              0.914,
                              1.143,
                              1.372,
                              1.600,
                              1.829,
                              2.057,
                              2.286,
                              2.515,
                              2.743,
                              2.972,
                              3.200,
                              3.429});
  std::vector<Real> spacer_K({1.06,
                              0.77,
                              1.41,
                              0.77,
                              1.41,
                              0.77,
                              1.41,
                              0.77,
                              1.41,
                              0.77,
                              1.41,
                              0.77,
                              1.41,
                              0.77,
                              1.41,
                              0.77});

  // Define the node placement along the z-axis.
  std::vector<Real> block_sizes;
  if (spacer_z.size() > 0 && spacer_z[0] != 0)
  {
    block_sizes.push_back(spacer_z[0]);
  }
  for (int i = 1; i < spacer_z.size(); i++)
  {
    block_sizes.push_back(spacer_z[i] - spacer_z[i - 1]);
  }
  constexpr Real GRID_TOL = 1e-4;
  if (spacer_z.size() > 0 && spacer_z.back() < 3.658 - GRID_TOL)
  {
    block_sizes.push_back(3.658 - spacer_z.back());
  }
  z_grid_.push_back(0.0);
  for (auto block_size : block_sizes)
  {
    int n = 1;
    while (n * max_dz_ < block_size)
      ++n;
    Real dz = block_size / n;
    for (int i = 0; i < n; i++)
      z_grid_.push_back(z_grid_.back() + dz);
  }
  nz_ = z_grid_.size() - 1;

  // Resize the gap-to-channel and channel-to-gap maps.
  unsigned int n_gaps = (nx_ - 1) * ny_ + (ny_ - 1) * nx_;
  gap_to_chan_map_.resize(n_gaps);
  gapnodes_.resize(n_gaps);
  chan_to_gap_map_.resize(nx_ * ny_);
  sign_id_crossflow_map_.resize(nx_ * ny_);
  gij_map_.resize(n_gaps);
  double possitive_flow = 1.0;
  double negative_flow = -1.0;

  // Index the east-west gaps.
  unsigned int i_gap = 0;
  for (unsigned int iy = 0; iy < ny_; iy++)
  {
    for (unsigned int ix = 0; ix < nx_ - 1; ix++)
    {
      unsigned int i_ch = nx_ * iy + ix;
      unsigned int j_ch = nx_ * iy + (ix + 1);
      gap_to_chan_map_[i_gap] = {i_ch, j_ch};
      chan_to_gap_map_[i_ch].push_back(i_gap);
      chan_to_gap_map_[j_ch].push_back(i_gap);
      sign_id_crossflow_map_[i_ch].push_back(possitive_flow);
      sign_id_crossflow_map_[j_ch].push_back(negative_flow);

      // make a gap size map
      if (iy == 0 || iy == ny_ - 1)
      {
        gij_map_[i_gap] = (pitch_ - rod_diameter_) / 2 + gap_;
      }
      else
      {
        gij_map_[i_gap] = (pitch_ - rod_diameter_);
      }

      ++i_gap;
    }
  }

  // Index the north-south gaps.
  for (unsigned int iy = 0; iy < ny_ - 1; iy++)
  {
    for (unsigned int ix = 0; ix < nx_; ix++)
    {
      unsigned int i_ch = nx_ * iy + ix;
      unsigned int j_ch = nx_ * (iy + 1) + ix;
      gap_to_chan_map_[i_gap] = {i_ch, j_ch};
      chan_to_gap_map_[i_ch].push_back(i_gap);
      chan_to_gap_map_[j_ch].push_back(i_gap);
      sign_id_crossflow_map_[i_ch].push_back(possitive_flow);
      sign_id_crossflow_map_[j_ch].push_back(negative_flow);

      // make a gap size map
      if (ix == 0 || ix == nx_ - 1)
      {
        gij_map_[i_gap] = (pitch_ - rod_diameter_) / 2 + gap_;
      }
      else
      {
        gij_map_[i_gap] = (pitch_ - rod_diameter_);
      }

      ++i_gap;
    }
  }

  // Reduce reserved memory in the channel-to-gap map.
  for (auto & gap : chan_to_gap_map_)
  {
    gap.shrink_to_fit();
  }
}

std::unique_ptr<MooseMesh>
SubChannelMesh::safeClone() const
{
  return libmesh_make_unique<SubChannelMesh>(*this);
}

void
SubChannelMesh::buildMesh()
{
  UnstructuredMesh & mesh = dynamic_cast<UnstructuredMesh &>(getMesh());
  mesh.clear();

  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  mesh.set_spatial_dimension(3);
  mesh.reserve_elem(nz_ * ny_ * nx_);
  mesh.reserve_nodes((nz_ + 1) * ny_ * nx_);
  nodes_.resize(nx_ * ny_);
  // Add the points in the shape of a rectilinear grid.  The grid is regular
  // on the xy-plane with a spacing of `pitch` between points.  The grid along
  // z is irregular to account for rod spacers.  Store pointers in the nodes_
  // array so we can keep track of which points are in which channels.
  unsigned int node_id = 0;
  for (unsigned int iy = 0; iy < ny_; iy++)
  {
    for (unsigned int ix = 0; ix < nx_; ix++)
    {
      int i_ch = nx_ * iy + ix;
      nodes_[i_ch].reserve(nz_);
      for (unsigned int iz = 0; iz < nz_ + 1; iz++)
      {
        nodes_[i_ch].push_back(
            mesh.add_point(Point(pitch_ * ix, pitch_ * iy, z_grid_[iz]), node_id++));
      }
    }
  }

  // Add the elements which in this case are 2-node edges that link each
  // subchannel's nodes vertically.
  unsigned int elem_id = 0;
  for (unsigned int iy = 0; iy < ny_; iy++)
  {
    for (unsigned int ix = 0; ix < nx_; ix++)
    {
      for (unsigned int iz = 0; iz < nz_; iz++)
      {
        Elem * elem = new Edge2;
        elem->set_id(elem_id++);
        elem = mesh.add_elem(elem);
        const int indx1 = ((nz_ + 1) * nx_) * iy + (nz_ + 1) * ix + iz;
        const int indx2 = ((nz_ + 1) * nx_) * iy + (nz_ + 1) * ix + (iz + 1);
        elem->set_node(0) = mesh.node_ptr(indx1);
        elem->set_node(1) = mesh.node_ptr(indx2);

        if (iz == 0)
          boundary_info.add_side(elem, 0, 0);
        if (iz == nz_ - 1)
          boundary_info.add_side(elem, 1, 1);
      }
    }
  }

  boundary_info.sideset_name(0) = "bottom";
  boundary_info.sideset_name(1) = "top";
  boundary_info.nodeset_name(0) = "bottom";
  boundary_info.nodeset_name(1) = "top";

  mesh.prepare_for_use();
}
