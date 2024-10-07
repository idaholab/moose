//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NearestPositionsPartitioner.h"

#include "MooseApp.h"
#include "DelimitedFileReader.h"
#include "KDTree.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/elem.h"

registerMooseObject("MooseApp", NearestPositionsPartitioner);

InputParameters
NearestPositionsPartitioner::validParams()
{
  InputParameters params = MoosePartitioner::validParams();

  params.addParam<FileName>(
      "positions_file",
      "CSV file providing the positions coordinates (x,y,z) in the first three columns. Note that "
      "Positions objects are not currently supported.");

  params.addClassDescription("Create a partition of the positions, then assign mesh elements based "
                             "on which position they are closest to.");

  return params;
}

NearestPositionsPartitioner::NearestPositionsPartitioner(const InputParameters & params)
  : MoosePartitioner(params), _mesh(*getCheckedPointerParam<MooseMesh *>("mesh"))
{
}

NearestPositionsPartitioner::~NearestPositionsPartitioner() {}

std::unique_ptr<Partitioner>
NearestPositionsPartitioner::clone() const
{
  return _app.getFactory().clone(*this);
}

void
NearestPositionsPartitioner::_do_partition(MeshBase & mesh, const unsigned int /*n*/)
{
  // Load the positions from the file
  const FileName positions_file = getParam<FileName>("positions_file");
  MooseUtils::DelimitedFileReader file(positions_file, &_communicator);
  file.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
  file.read();
  const std::vector<Point> & positions_unindexed = file.getDataAsPoints();
  const auto n_positions = positions_unindexed.size();

  // Keep track of the index of each point, in case some positions are duplicated
  std::vector<std::pair<unsigned int, Point>> positions(n_positions);
  for (const auto i : index_range(positions))
    positions[i] = std::make_pair(i, positions_unindexed[i]);

  // Re-use this vector for neighbor searches
  std::vector<std::size_t> return_index(2);

  // Lloyd's algorithm:
  // 1 - Build an initial close to even split
  std::vector<std::vector<std::pair<unsigned int, Point>>> positions_array;
  const auto n_bins = mesh.n_partitions();
  positions_array.resize(n_bins);
  for (auto & position_bin : positions_array)
    // the number of bins has to be respected, so we may need to have a final bin with less
    // positions
    position_bin.reserve(ceil(float(n_positions) / n_bins));
  for (const auto i : index_range(positions))
    positions_array[int(float(i) / n_positions * n_bins)].push_back(positions[i]);

  if (n_bins > n_positions)
    mooseError("Not implemented for more MPI processes than positions");

  // 2 - Iteration
  std::vector<Point> centroids(n_bins);
  std::vector<unsigned int> group_sizes(n_bins);
  bool converged = false;
  unsigned int iter = 0;
  while (!converged)
  {
    // Compute the centroids of each group
    for (const auto bin_index : index_range(positions_array))
    {
      for (const auto & position : positions_array[bin_index])
        centroids[bin_index] += position.second;

      // Normalize by actual number of points
      const auto bin_size = positions_array[bin_index].size();
      centroids[bin_index](0) = centroids[bin_index](0) / bin_size;
      centroids[bin_index](1) = centroids[bin_index](1) / bin_size;
      centroids[bin_index](2) = centroids[bin_index](2) / bin_size;

      // Keep track of the sizes of each group
      group_sizes[bin_index] = bin_size;
    }

    // Create a KD Tree for the centroids
    KDTree centroids_kd_tree(centroids, 1);

    // Re-assign points based on how close they are to each centroid
    unsigned int num_swaps = 0;
    unsigned int num_swaps_skipped = 0;
    for (const auto j : index_range(positions_array))
    {
      // no need to resort positions that were moved once, so we keep track of the original
      // sizes of the positions groups in group_sizes
      // k_vec keeps track of actual index in positions bin vector
      auto k_vec = 0;
      for (auto k = 0; k < group_sizes[j]; k++)
      {
        const auto position = positions_array[j][k_vec];
        centroids_kd_tree.neighborSearch(position.second, 2, return_index);

        // Note: to get a more even partitioning of positions, we can play on the swap rules
        // - only swap the furthest from the centroid
        // - only swap up to the expected even size split
        // Swap positions between bins. Current rules:
        // - don't swap if the receiving bin is 25% bigger already
        // - try 2nd largest bin
        if (return_index[0] != j)
        {
          if (positions_array[return_index[0]].size() < 1.25 * positions_array[j].size())
          {
            num_swaps++;
            positions_array[return_index[0]].push_back(position);
            positions_array[j].erase(positions_array[j].begin() + k_vec);
            k_vec--;

            // recompute target group centroid
            const auto bin_size = positions_array[return_index[0]].size();
            const auto bin_index = return_index[0];
            centroids[bin_index](0) =
                (centroids[bin_index](0) * bin_size + position.second(0)) / (bin_size + 1);
            centroids[bin_index](1) =
                (centroids[bin_index](1) * bin_size + position.second(1)) / (bin_size + 1);
            centroids[bin_index](2) =
                (centroids[bin_index](2) * bin_size + position.second(2)) / (bin_size + 1);
          }
          // use the second best group if can't use the first one
          else if (positions_array[return_index[1]].size() < 1.25 * positions_array[j].size())
          {
            num_swaps++;
            positions_array[return_index[1]].push_back(position);
            positions_array[j].erase(positions_array[j].begin() + k_vec);
            k_vec--;

            // recompute target group centroid
            const auto bin_size = positions_array[return_index[1]].size();
            const auto bin_index = return_index[1];
            centroids[bin_index](0) =
                (centroids[bin_index](0) * bin_size + position.second(0)) / (bin_size + 1);
            centroids[bin_index](1) =
                (centroids[bin_index](1) * bin_size + position.second(1)) / (bin_size + 1);
            centroids[bin_index](2) =
                (centroids[bin_index](2) * bin_size + position.second(2)) / (bin_size + 1);
          }
          else
            num_swaps_skipped++;
        }
        k_vec++;
      }
    }

    // Examine convergence
    _console << "Iteration " << iter << " # swaps made : " << num_swaps
             << " # swaps skipped : " << num_swaps_skipped << std::endl;
    if (num_swaps == 0)
      converged = true;
    iter++;
    if (iter > 100)
    {
      converged = true;
      mooseWarning("Lloyd's algorithm for sorting positions into " + std::to_string(n_bins) +
                   " groups (=number of MPI processes) did not converge after 100 iterations");
    }
  }

  // Report on position assignment performance
  for (const auto position_bin_index : make_range(n_bins))
    _console << "Bin " << position_bin_index << " : " << positions_array[position_bin_index].size()
             << " positions" << std::endl;

  // Keep track of the partition assignment of each position
  std::vector<unsigned int> partition_assigment(n_positions);
  for (const auto position_bin_index : make_range(n_bins))
    for (const auto & [position_index, pt] : positions_array[position_bin_index])
    {
      libmesh_ignore(pt);
      partition_assigment[position_index] = position_bin_index;
    }

  // Build the KDTree from the positions, which will help to get the index
  KDTree positions_kd_tree(positions_unindexed, 1);
  return_index.resize(1);

  // Loop over all of the elements in the given mesh to assign them to a partition
  for (auto & elem_ptr : mesh.active_element_ptr_range())
  {
    // Find the element centroid
    auto centroid = elem_ptr->vertex_average();

    // Find the position nearest the point
    positions_kd_tree.neighborSearch(centroid, 1, return_index);

    // Assign the position's group index to current element
    elem_ptr->processor_id() = partition_assigment[return_index[0]];
  }
}
