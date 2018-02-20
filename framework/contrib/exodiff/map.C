// Copyright(C) 2008-2017 National Technology & Engineering Solutions
// of Sandia, LLC (NTESS).  Under the terms of Contract DE-NA0003525 with
// NTESS, the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//
//     * Neither the name of NTESS nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include <cfloat>
#include <cstdlib>
#include <iomanip>

#include "ED_SystemInterface.h"
#include "Tolerance.h"
#include "exoII_read.h"
#include "exo_block.h"
#include "iqsort.h"
#include "smart_assert.h"
#include "util.h"

namespace {
  double find_range(const double *x, size_t num_nodes);

  template <typename INT>
  INT Find(double x0,
           double y0,
           double z0,
           double * x,
           double * y,
           double * z,
           INT * id,
           size_t N,
           int dim,
           bool ignore_dups);

  template <typename INT>
  void Compute_Node_Map(INT *& node_map, ExoII_Read<INT> & file1, ExoII_Read<INT> & file2);
  } // namespace

  template <typename INT>
  void
  Compute_Maps(INT *& node_map, INT *& elmt_map, ExoII_Read<INT> & file1, ExoII_Read<INT> & file2)
  {
    SMART_ASSERT(file1.Open());
    SMART_ASSERT(file2.Open());

    size_t num_nodes = file1.Num_Nodes();
    size_t num_elmts = file1.Num_Elmts();
    int dim = file1.Dimension();

    //  ********************  elements  ********************  //

    // Load global ids (0-offset) into id array.
    auto id = new INT[num_elmts];
    {
      for (size_t e = 0; e < num_elmts; ++e)
      {
        id[e] = e;
      }
    }

    // Get map storage.
    node_map = new INT[num_nodes];
    SMART_ASSERT(node_map != nullptr);
    {
      for (size_t i = 0; i < num_nodes; ++i)
      {
        node_map[i] = -1;
      }
    }
    elmt_map = new INT[num_elmts];
    SMART_ASSERT(elmt_map != nullptr);

    // Create storage for midpoints.
    double *x2 = nullptr, *y2 = nullptr, *z2 = nullptr;
    x2 = new double[num_elmts];
    SMART_ASSERT(x2 != nullptr);
    if (dim > 1)
    {
      y2 = new double[num_elmts];
      SMART_ASSERT(y2 != nullptr);
    }
    if (dim > 2)
    {
      z2 = new double[num_elmts];
      SMART_ASSERT(z2 != nullptr);
    }

    // Load coordinates for file 2 and get pointers to them.
    file2.Load_Nodal_Coordinates();
    const double * x2_f = (double *)file2.X_Coords();
    const double * y2_f = (double *)file2.Y_Coords();
    const double * z2_f = (double *)file2.Z_Coords();

    // Load connectivities for all blocks in second file.
    file2.Load_Elmt_Block_Descriptions();

    {
      // Compute midpoints of each element and place into x,y,z arrays.
      size_t num_blocks = file2.Num_Elmt_Blocks(), num_elmts_in_block, num_nodes_per_elmt, e = 0;
      double sum_x, sum_y, sum_z;
      for (size_t b = 0; b < num_blocks; ++b)
      {
        const Exo_Block<INT> * block = file2.Get_Elmt_Block_by_Index(b);
        num_elmts_in_block = block->Size();
        num_nodes_per_elmt = block->Num_Nodes_per_Elmt();
        for (size_t i = 0; i < num_elmts_in_block; ++i)
        {
          const INT * conn = block->Connectivity(i); // Connectivity for element i.
          sum_x = 0.0;
          sum_y = 0.0;
          sum_z = 0.0;
          for (size_t j = 0; j < num_nodes_per_elmt; ++j)
          {
            sum_x += x2_f[conn[j] - 1];
            if (dim > 1)
            {
              sum_y += y2_f[conn[j] - 1];
            }
            if (dim > 2)
            {
              sum_z += z2_f[conn[j] - 1];
            }
          }
          x2[e] = sum_x / static_cast<double>(num_nodes_per_elmt);
          if (dim > 1)
          {
            y2[e] = sum_y / static_cast<double>(num_nodes_per_elmt);
          }
          if (dim > 2)
          {
            z2[e] = sum_z / static_cast<double>(num_nodes_per_elmt);
          }

          ++e;
        }
      }
    }

    // Sort by x value.
    index_qsort(x2, id, num_elmts);

#if 0
  std::cout << "******************  elmts  ******************** \n";
  {for (size_t i = 0; i < num_elmts; ++i)
      std::cout << i << ")\t"
		<< x2[id[i]] << "\t"
		<< y2[id[i]] << "\t"
		<< z2[id[i]] << "\t" << id[i] << '\n';}
  std::cout << "******************  elmts  ******************** \n";
#endif
  //  Load and get nodal coordinates for first file.
  file1.Load_Nodal_Coordinates();
  const double * x1_f = (double *)file1.X_Coords();
  const double * y1_f = (double *)file1.Y_Coords();
  const double * z1_f = (double *)file1.Z_Coords();

  // Cannot ignore the comparisons, so make sure the coord_tol_type
  // is not -1 which is "ignore"
  TOLERANCE_TYPE_enum save_tolerance_type = interface.coord_tol.type;
  if (save_tolerance_type == IGNORE)
  {
    interface.coord_tol.type = ABSOLUTE;
  }

  // Match elmts in first file to their corresponding elmts in second.
  size_t num_blocks = file1.Num_Elmt_Blocks();
  size_t num_elmts_in_block;
  size_t num_nodes_per_elmt;
  size_t e1 = 0;
  size_t e2 = 0;
  INT sort_idx;
  double mid_x, mid_y, mid_z;

  for (size_t b = 0; b < num_blocks; ++b)
  {
    const Exo_Block<INT> * block1 = file1.Get_Elmt_Block_by_Index(b);
    file1.Load_Elmt_Block_Description(b);
    num_elmts_in_block = block1->Size();
    num_nodes_per_elmt = block1->Num_Nodes_per_Elmt();
    for (size_t i = 0; i < num_elmts_in_block; ++i)
    {
      // Connectivity for element i.
      const INT * conn1 = block1->Connectivity(i);

      // Compute midpoint.
      mid_x = 0.0;
      mid_y = 0.0;
      mid_z = 0.0;

      for (size_t j = 0; j < num_nodes_per_elmt; ++j)
      {
        SMART_ASSERT(conn1[j] >= 1 && conn1[j] <= (INT)num_nodes);
        mid_x += x1_f[conn1[j] - 1];
        if (dim > 1)
        {
          mid_y += y1_f[conn1[j] - 1];
        }
        if (dim > 2)
        {
          mid_z += z1_f[conn1[j] - 1];
        }
      }
      mid_x /= static_cast<double>(num_nodes_per_elmt);
      if (dim > 1)
      {
        mid_y /= static_cast<double>(num_nodes_per_elmt);
      }
      if (dim > 2)
      {
        mid_z /= static_cast<double>(num_nodes_per_elmt);
      }

      // Locate midpoint in sorted array.
      sort_idx = Find(mid_x, mid_y, mid_z, x2, y2, z2, id, num_elmts, dim, interface.ignore_dups);

      if (sort_idx < 0)
      {
        ERROR("Files are different (couldn't match element "
              << (i + 1) << " from block " << file1.Block_Id(b) << " from first file to second)\n");
        exit(1);
      }
      e2 = id[sort_idx];

      // Assign element map for this element.
      elmt_map[e1] = e2;

      {
        // Determine the block and elmt index of matched element.
        int b2;
        size_t l2;
        file2.Global_to_Block_Local(e2 + 1, b2, l2);

        const Exo_Block<INT> * block2 = file2.Get_Elmt_Block_by_Index(b2);
        SMART_ASSERT(block2 != nullptr);

        // Check that the element types are the same.
        if (num_nodes_per_elmt != block2->Num_Nodes_per_Elmt())
        {
          ERROR("Files are different.\n"
                << " In File 1: Element " << (i + 1) << " in Block " << file1.Block_Id(b) << " has "
                << num_nodes_per_elmt << " and\n"
                << " In File 2: Element " << (l2 + 1) << " in Block " << file2.Block_Id(b2)
                << " has " << block2->Num_Nodes_per_Elmt() << '\n');
          exit(1);
        }

        // Get connectivity for file2 element.
        const INT * conn2 = block2->Connectivity(l2);

        // Match each node in the first elmt with a node in the second
        // and assign node_map.
        for (size_t ln1 = 0; ln1 < num_nodes_per_elmt; ++ln1)
        {
          // Grab coordinate of node in first file.
          double x1_val = x1_f[conn1[ln1] - 1];
          double y1_val = dim > 1 ? y1_f[conn1[ln1] - 1] : 0.0;
          double z1_val = dim > 2 ? z1_f[conn1[ln1] - 1] : 0.0;

          size_t found = 0;
          for (size_t ln2 = 0; ln2 < num_nodes_per_elmt; ++ln2)
          {
            // Grab coordinate of node in second file.
            double x2_val = x2_f[conn2[ln2] - 1];
            double y2_val = dim > 1 ? y2_f[conn2[ln2] - 1] : 0.0;
            double z2_val = dim > 2 ? z2_f[conn2[ln2] - 1] : 0.0;

            if (!interface.coord_tol.Diff(x1_val, x2_val) &&
                !interface.coord_tol.Diff(y1_val, y2_val) &&
                !interface.coord_tol.Diff(z1_val, z2_val))
            {
              // assert that if this node has been given a map
              // previously, that it agrees with the latest
              // assignment.
              if (node_map[conn1[ln1] - 1] >= 0 && node_map[conn1[ln1] - 1] != conn2[ln2] - 1)
              {

                if (!interface.ignore_dups)
                {
                  // Node in file 1.
                  INT node1 = conn1[ln1];
                  double x1a = x1_f[node1 - 1];
                  double y1a = dim >= 2 ? y1_f[node1 - 1] : 0.0;
                  double z1a = dim >= 3 ? z1_f[node1 - 1] : 0.0;

                  // Node in file 2 that was already mapped to node 1 in file 1
                  INT n1 = node_map[conn1[ln1] - 1] + 1;
                  double x2a = x2_f[n1 - 1];
                  double y2a = dim >= 2 ? y2_f[n1 - 1] : 0.0;
                  double z2a = dim >= 3 ? z2_f[n1 - 1] : 0.0;

                  // Node in file 2 that is now being mapped to node 1 in file 1
                  INT n2 = conn2[ln2];
                  double x2b = x2_f[n2 - 1];
                  double y2b = dim >= 2 ? y2_f[n2 - 1] : 0.0;
                  double z2b = dim >= 3 ? z2_f[n2 - 1] : 0.0;

                  SMART_ASSERT(!interface.coord_tol.Diff(x2a, x2b) &&
                               !interface.coord_tol.Diff(y2a, y2b) &&
                               !interface.coord_tol.Diff(z2a, z2b));
                  ERROR("No unique node mapping possible.\n"
                        << "\tFile 1, Node " << node1 << " at (" << x1a << ", " << y1a << ", "
                        << z1a << ") maps to both:\n"
                        << "\tFile 2, Node " << n1 << " at (" << x2a << ", " << y2a << ", " << z2a
                        << ") and\n"
                        << "\tFile 2, Node " << n2 << " at (" << x2b << ", " << y2b << ", " << z2b
                        << ")\n\n");
                  exit(1);
                }
                found = 1;
                break;
              }
              node_map[conn1[ln1] - 1] = conn2[ln2] - 1;
              found = 1;
              break;
            }
          }
          if (!found)
          {
            std::ostringstream out;
            out << "\nexodiff: ERROR: Cannot find a match for node at position " << ln1 + 1
                << " in first element.\n"
                << "\tFile 1: Element " << (i + 1) << " in Block " << file1.Block_Id(b)
                << " nodes:\n";
            for (size_t l1 = 0; l1 < num_nodes_per_elmt; ++l1)
            {
              double x_val = x1_f[conn1[l1] - 1];
              double y_val = dim > 1 ? y1_f[conn1[l1] - 1] : 0.0;
              double z_val = dim > 2 ? z1_f[conn1[l1] - 1] : 0.0;
              out << "\t(" << l1 + 1 << ")\t" << conn1[l1] << "\t" << std::setprecision(9) << x_val
                  << "\t" << y_val << "\t" << z_val << "\n";
            }
            out << "\tFile 2: Element " << (l2 + 1) << " in Block " << file1.Block_Id(b)
                << " nodes:\n";
            for (size_t l3 = 0; l3 < num_nodes_per_elmt; ++l3)
            {
              double x_val = x2_f[conn2[l3] - 1];
              double y_val = dim > 1 ? y2_f[conn2[l3] - 1] : 0.0;
              double z_val = dim > 2 ? z2_f[conn2[l3] - 1] : 0.0;
              out << "\t(" << l3 + 1 << ")\t" << conn2[l3] << "\t" << std::setprecision(9) << x_val
                  << "\t" << y_val << "\t" << z_val << "\n";
            }
            out << "Coordinates compared using tolerance: " << interface.coord_tol.value << " ("
                << interface.coord_tol.typestr() << "), floor: " << interface.coord_tol.floor
                << "\n";
            ERR_OUT(out);
            exit(1);
          }
        } // End of local node loop on file1's element.
      }   // End of local node search block.

      ++e1;

    } // End of loop on elements in file1 element block.

    file1.Free_Elmt_Block(b);

  } // End of loop on file1 blocks.

  // Check that all nodes in the file have been matched...  If any
  // unmatched nodes are found, then perform a node-based matching
  // algorithm...
  for (size_t i = 0; i < num_nodes; i++)
  {
    if (node_map[i] < 0) {
      Compute_Node_Map(node_map, file1, file2);
      break;
    }
  }

  file1.Free_Nodal_Coordinates();
  file2.Free_Nodal_Coordinates();
  file2.Free_Elmt_Blocks();

  if (x2 != nullptr)
  {
    delete[] x2;
  }
  if (y2 != nullptr)
  {
    delete[] y2;
  }
  if (z2 != nullptr)
  {
    delete[] z2;
  }
  if (id != nullptr)
  {
    delete[] id;
  }

  interface.coord_tol.type = save_tolerance_type;
}

template <typename INT>
void
Compute_Partial_Maps(INT *& node_map,
                     INT *& elmt_map,
                     ExoII_Read<INT> & file1,
                     ExoII_Read<INT> & file2)
{
  SMART_ASSERT(file1.Open());
  SMART_ASSERT(file2.Open());

  size_t num_nodes1 = file1.Num_Nodes();
  size_t num_elmts1 = file1.Num_Elmts();

  size_t num_nodes2 = file2.Num_Nodes();
  size_t num_elmts2 = file2.Num_Elmts();
  int dim = file1.Dimension();
  SMART_ASSERT(dim == file2.Dimension());

  //  ********************  elements  ********************  //

  // Load global ids (0-offset) into id array.
  auto id2 = new INT[num_elmts2];
  {
    for (size_t e = 0; e < num_elmts2; ++e)
    {
      id2[e] = e;
    }
  }

  // Get map storage.
  node_map = new INT[num_nodes1];
  SMART_ASSERT(node_map != nullptr);
  {
    for (size_t i = 0; i < num_nodes1; ++i)
    {
      node_map[i] = -1;
    }
  }
  elmt_map = new INT[num_elmts1];
  SMART_ASSERT(elmt_map != nullptr);
  {
    for (size_t i = 0; i < num_elmts1; ++i)
    {
      elmt_map[i] = -1;
    }
  }

  // Create storage for midpoints.
  double *x2 = nullptr, *y2 = nullptr, *z2 = nullptr;
  x2 = new double[num_elmts2];
  SMART_ASSERT(x2 != nullptr);
  if (dim > 1)
  {
    y2 = new double[num_elmts2];
    SMART_ASSERT(y2 != nullptr);
  }
  if (dim > 2)
  {
    z2 = new double[num_elmts2];
    SMART_ASSERT(z2 != nullptr);
  }

  // Load coordinates for file 2 and get pointers to them.
  file2.Load_Nodal_Coordinates();
  const double * x2_f = (double *)file2.X_Coords();
  const double * y2_f = (double *)file2.Y_Coords();
  const double * z2_f = (double *)file2.Z_Coords();

  // Load connectivities for all blocks in second file.
  file2.Load_Elmt_Block_Descriptions();

  {
    // Compute midpoints of each element and place into x,y,z arrays.
    size_t num_blocks2 = file2.Num_Elmt_Blocks(), num_elmts_in_block, num_nodes_per_elmt, e = 0;
    double sum_x, sum_y, sum_z;
    for (size_t b = 0; b < num_blocks2; ++b)
    {
      const Exo_Block<INT> * block = file2.Get_Elmt_Block_by_Index(b);
      num_elmts_in_block = block->Size();
      num_nodes_per_elmt = block->Num_Nodes_per_Elmt();
      for (size_t i = 0; i < num_elmts_in_block; ++i)
      {
        const INT * conn = block->Connectivity(i); // Connectivity for element i.
        sum_x = 0.0;
        sum_y = 0.0;
        sum_z = 0.0;
        for (size_t j = 0; j < num_nodes_per_elmt; ++j)
        {
          sum_x += x2_f[conn[j] - 1];
          if (dim > 1)
          {
            sum_y += y2_f[conn[j] - 1];
          }
          if (dim > 2)
          {
            sum_z += z2_f[conn[j] - 1];
          }
        }
        x2[e] = sum_x / static_cast<double>(num_nodes_per_elmt);
        if (dim > 1)
        {
          y2[e] = sum_y / static_cast<double>(num_nodes_per_elmt);
        }
        if (dim > 2)
        {
          z2[e] = sum_z / static_cast<double>(num_nodes_per_elmt);
        }

        ++e;
      }
    }
  }

  // Sort by x value.
  index_qsort(x2, id2, num_elmts2);

#if 0
  std::cout << "******************  elmts  ******************** \n";
  {for (size_t i = 0; i < num_elmts; ++i)
    std::cout << i << ")\t"
	      << x2[id[i]] << "\t"
	      << y2[id[i]] << "\t"
	      << z2[id[i]] << "\t" << id[i] << '\n';}
  std::cout << "******************  elmts  ******************** \n";
#endif
  //  Load and get nodal coordinates for first file.
  file1.Load_Nodal_Coordinates();
  const double * x1_f = (double *)file1.X_Coords();
  const double * y1_f = (double *)file1.Y_Coords();
  const double * z1_f = (double *)file1.Z_Coords();

  // Cannot ignore the comparisons, so make sure the coord_tol_type
  // is not -1 which is "ignore"
  TOLERANCE_TYPE_enum save_tolerance_type = interface.coord_tol.type;
  if (save_tolerance_type == IGNORE)
  {
    interface.coord_tol.type = ABSOLUTE;
  }

  // Match elmts in first file to their corresponding elmts in second.
  size_t num_blocks1 = file1.Num_Elmt_Blocks();
  size_t num_elmts_in_block;
  size_t num_nodes_per_elmt;
  size_t e1 = 0;
  size_t e2 = 0;
  INT    sort_idx;
  double mid_x, mid_y, mid_z;

  bool first = true;
  size_t unmatched = 0;
  for (size_t b = 0; b < num_blocks1; ++b)
  {
    const Exo_Block<INT> * block1 = file1.Get_Elmt_Block_by_Index(b);
    file1.Load_Elmt_Block_Description(b);
    num_elmts_in_block = block1->Size();
    num_nodes_per_elmt = block1->Num_Nodes_per_Elmt();
    for (size_t i = 0; i < num_elmts_in_block; ++i)
    {
      // Connectivity for element i.
      const INT * conn1 = block1->Connectivity(i);

      // Compute midpoint.
      mid_x = 0.0;
      mid_y = 0.0;
      mid_z = 0.0;

      for (size_t j = 0; j < num_nodes_per_elmt; ++j) {
        SMART_ASSERT(conn1[j] >= 1 && conn1[j] <= (INT)num_nodes1);
        mid_x += x1_f[conn1[j] - 1];
        if (dim > 1)
        {
          mid_y += y1_f[conn1[j] - 1];
        }
        if (dim > 2)
        {
          mid_z += z1_f[conn1[j] - 1];
        }
      }
      mid_x /= static_cast<double>(num_nodes_per_elmt);
      if (dim > 1)
      {
        mid_y /= static_cast<double>(num_nodes_per_elmt);
      }
      if (dim > 2)
      {
        mid_z /= static_cast<double>(num_nodes_per_elmt);
      }

      // Locate midpoint in sorted array.
      sort_idx = Find(mid_x, mid_y, mid_z, x2, y2, z2, id2, num_elmts2, dim, interface.ignore_dups);
      if (sort_idx < 0) {
        unmatched++;
        if (first && interface.show_unmatched)
        {
          std::cout << "exodiff: Doing Partial Comparison: No Match for (b.e):\n";
        }
        first = false;
        if (interface.show_unmatched)
        {
          std::cout << file1.Block_Id(b) << "." << (i + 1) << ", ";
        }
      }
      else
      {
        e2 = id2[sort_idx];
        elmt_map[e1] = e2;

        // Assign element map for this element.

        // Determine the block and elmt index of matched element.
        int b2;
        size_t l2;
        file2.Global_to_Block_Local(e2 + 1, b2, l2);

        const Exo_Block<INT> * block2 = file2.Get_Elmt_Block_by_Index(b2);
        SMART_ASSERT(block2 != nullptr);

        // Check that the element types are the same.
        if (num_nodes_per_elmt != block2->Num_Nodes_per_Elmt())
        {
          ERROR("Files are different.\n"
                << " In File 1: Element " << (i + 1) << " in Block " << file1.Block_Id(b) << " has "
                << num_nodes_per_elmt << " and\n"
                << " In File 2: Element " << (l2 + 1) << " in Block " << file2.Block_Id(b2)
                << " has " << block2->Num_Nodes_per_Elmt() << '\n');
          exit(1);
        }

        // Get connectivity for file2 element.
        const INT * conn2 = block2->Connectivity(l2);

        // Match each node in the first elmt with a node in the second
        // and assign node_map.
        for (size_t ln1 = 0; ln1 < num_nodes_per_elmt; ++ln1)
        {
          // Grab coordinate of node in first file.
          double x1_val = x1_f[conn1[ln1] - 1];
          double y1_val = dim > 1 ? y1_f[conn1[ln1] - 1] : 0.0;
          double z1_val = dim > 2 ? z1_f[conn1[ln1] - 1] : 0.0;
          size_t found = 0;
          for (size_t ln2 = 0; ln2 < num_nodes_per_elmt; ++ln2)
          {
            // Grab coordinate of node in second file.
            double x2_val = x2_f[conn2[ln2] - 1];
            double y2_val = dim > 1 ? y2_f[conn2[ln2] - 1] : 0.0;
            double z2_val = dim > 2 ? z2_f[conn2[ln2] - 1] : 0.0;

            if (!interface.coord_tol.Diff(x1_val, x2_val) &&
                !interface.coord_tol.Diff(y1_val, y2_val) &&
                !interface.coord_tol.Diff(z1_val, z2_val))
            {
              node_map[conn1[ln1] - 1] = conn2[ln2] - 1;
              found = 1;
              break;
            }
          }
          if (!found) {
            std::ostringstream out;
            out << "\nexodiff: ERROR: Cannot find a match for node at position " << ln1 + 1
                << " in first element.\n"
                << "\tFile 1: Element " << (i + 1) << " in Block " << file1.Block_Id(b)
                << " nodes:\n";
            for (size_t l1 = 0; l1 < num_nodes_per_elmt; ++l1)
            {
              double x_val = x1_f[conn1[l1] - 1];
              double y_val = dim > 1 ? y1_f[conn1[l1] - 1] : 0.0;
              double z_val = dim > 2 ? z1_f[conn1[l1] - 1] : 0.0;
              out << "\t(" << l1 + 1 << ")\t" << conn1[l1] << "\t" << std::setprecision(9) << x_val
                  << "\t" << y_val << "\t" << z_val << "\n";
            }
            out << "\tFile 2: Element " << (l2 + 1) << " in Block " << file1.Block_Id(b)
                << " nodes:\n";
            for (size_t l3 = 0; l3 < num_nodes_per_elmt; ++l3)
            {
              double x_val = x2_f[conn2[l3] - 1];
              double y_val = dim > 1 ? y2_f[conn2[l3] - 1] : 0.0;
              double z_val = dim > 2 ? z2_f[conn2[l3] - 1] : 0.0;
              out << "\t(" << l3 + 1 << ")\t" << conn2[l3] << "\t" << std::setprecision(9) << x_val
                  << "\t" << y_val << "\t" << z_val << "\n";
            }
            out << "Coordinates compared using tolerance: " << interface.coord_tol.value << " ("
                << interface.coord_tol.typestr() << "), floor: " << interface.coord_tol.floor
                << "\n";
            ERR_OUT(out);
            exit(1);
          }
        } // End of local node loop on file1's element.
      }   // End of local node search block.

      ++e1;

    } // End of loop on elements in file1 element block.
    file1.Free_Elmt_Block(b);

  } // End of loop on file1 blocks.
  if (!first) {
    std::cout << "\nPartial Map selected -- " << unmatched << " elements unmatched\n";
  }
  else
  {
    if (num_elmts1 == num_elmts2 && num_nodes1 == num_nodes2)
    {
      std::cout
          << "exodiff: INFO .. Partial Map was specfied, but not needed.  All elements matched.\n";
    }
  }

  // Check that all nodes in the file have been matched...  If any
  // unmatched nodes are found, then perform a node-based matching
  // algorithm...
  //   for (size_t i=0; i < num_nodes; i++) {
  //     if (node_map[i] < 0) {
  //       Compute_Node_Map(node_map, file1, file2);
  //       break;
  //     }
  //   }

  file1.Free_Nodal_Coordinates();
  file2.Free_Nodal_Coordinates();
  file2.Free_Elmt_Blocks();

  if (x2 != nullptr)
  {
    delete[] x2;
  }
  if (y2 != nullptr)
  {
    delete[] y2;
  }
  if (z2 != nullptr)
  {
    delete[] z2;
  }
  if (id2 != nullptr)
  {
    delete[] id2;
  }

  interface.coord_tol.type = save_tolerance_type;
}

namespace {
template <typename INT>
bool
check_sort(const INT * map, size_t count)
{
  for (size_t i = 1; i < count; i++)
  {
    if (map[i - 1] > map[i])
    {
      return true;
    }
  }
  return false;
  }

  template <typename INT>
  bool
  internal_compute_maps(INT * map,
                        const INT * file1_id_map,
                        const INT * file2_id_map,
                        size_t count,
                        const char * type)
  {
    std::vector<INT> id1;
    id1.reserve(count);
    std::vector<INT> id2;
    id2.reserve(count);
    for (size_t i = 0; i < count; i++)
    {
      id1.push_back(i);
      id2.push_back(i);
    }

    // Check whether sorting needed...
    bool sort1_needed = check_sort(file1_id_map, count);
    if (sort1_needed)
    {
      index_qsort(file1_id_map, &id1[0], count);
    }

    bool sort2_needed = check_sort(file2_id_map, count);
    if (sort2_needed)
    {
      index_qsort(file2_id_map, &id2[0], count);
    }

    for (size_t i = 0; i < count; i++)
    {
      if (file1_id_map[id1[i]] == file2_id_map[id2[i]]) {
        map[id1[i]] = id2[i];
      }
      else
      {
        ERROR("Unable to match " << type << " " << file1_id_map[id1[i]] << " in first file with "
                                 << type << " in second file.\n");
        exit(1);
      }
    }

    // See if there is any mapping happening...
    bool mapped = false;
    for (INT i = 0; i < (INT)count; i++)
    {
      if (i != map[i]) {
        mapped = true;
        break;
      }
    }
    return mapped;
  }
  } // namespace

  template <typename INT>
  void
  Compute_FileId_Maps(INT *& node_map,
                      INT *& elmt_map,
                      ExoII_Read<INT> & file1,
                      ExoII_Read<INT> & file2)
  {
    // Compute map of nodes and elements in file1 to nodes and elements in file2
    // Use the internal exodus node and element number maps in file1 and file2 to
    // do the matching.  Currently assume (and verify) that number of nodes and
    // elements match in the two files.

    SMART_ASSERT(file1.Open());
    SMART_ASSERT(file2.Open());

    {
      size_t num_nodes = file1.Num_Nodes();
      SMART_ASSERT(num_nodes == file2.Num_Nodes());

      node_map = new INT[num_nodes];
      SMART_ASSERT(node_map != nullptr);
      file1.Load_Node_Map();
      file2.Load_Node_Map();
      const INT * node_id_map1 = file1.Get_Node_Map();
      const INT * node_id_map2 = file2.Get_Node_Map();

      if (!internal_compute_maps(node_map, node_id_map1, node_id_map2, num_nodes, "node"))
      {
        delete[] node_map;
        node_map = nullptr;
      }
    }

    {
      size_t num_elmts = file1.Num_Elmts();
      SMART_ASSERT(num_elmts == file2.Num_Elmts());
      elmt_map = new INT[num_elmts];
      SMART_ASSERT(elmt_map != nullptr);
      file1.Load_Elmt_Map();
      file2.Load_Elmt_Map();
      const INT * elem_id_map1 = file1.Get_Elmt_Map();
      const INT * elem_id_map2 = file2.Get_Elmt_Map();

      if (!internal_compute_maps(elmt_map, elem_id_map1, elem_id_map2, num_elmts, "element"))
      {
        delete[] elmt_map;
        elmt_map = nullptr;
      }
    }
}

template <typename INT>
void
Dump_Maps(const INT * node_map, const INT * elmt_map, ExoII_Read<INT> & file1)
{
  size_t ijk;
  std::cout << "\n=== node number map (file1 -> file2) local ids\n";
  bool one_to_one = true;
  if (node_map != nullptr)
  {
    for (ijk = 0; ijk < file1.Num_Nodes(); ++ijk)
    {
      if ((INT)ijk != node_map[ijk])
      {
        one_to_one = false;
        break;
      }
    }
  }
  if (!one_to_one) {
    for (ijk = 0; ijk < file1.Num_Nodes(); ++ijk)
    {
      std::cout << (ijk + 1) << " -> " << (node_map[ijk] + 1) << "\n";
    }
  }
  else
  {
    std::cout << " *** Node map is one-to-one\n";
  }

  std::cout << "\n=== element number map (file1 -> file2) local ids\n";
  one_to_one = true;
  if (elmt_map != nullptr)
  {
    for (ijk = 0; ijk < file1.Num_Elmts(); ++ijk)
    {
      if ((INT)ijk != elmt_map[ijk])
      {
        one_to_one = false;
        break;
      }
    }
  }
  if (!one_to_one) {
    for (ijk = 0; ijk < file1.Num_Elmts(); ++ijk)
    {
      std::cout << (ijk + 1) << " -> " << (elmt_map[ijk] + 1) << "\n";
    }
  }
  else
  {
    std::cout << " *** Element map is one-to-one\n";
  }
  std::cout << "===\n";
}

template <typename INT>
bool
Check_Maps(const INT * node_map,
           const INT * elmt_map,
           const ExoII_Read<INT> & file1,
           const ExoII_Read<INT> & file2)
{
  if (file1.Num_Nodes() != file2.Num_Nodes()) {
    return false;
  }

  if (file1.Num_Elmts() != file2.Num_Elmts()) {
    return false;
  }

  if (node_map != nullptr)
  {
    for (size_t ijk = 0; ijk < file1.Num_Nodes(); ++ijk) {
      if ((INT)ijk != node_map[ijk]) {
        return false;
      }
    }
  }

  if (elmt_map != nullptr)
  {
    for (size_t ijk = 0; ijk < file1.Num_Elmts(); ++ijk) {
      if ((INT)ijk != elmt_map[ijk]) {
        return false;
      }
    }
  }
  // All maps are one-to-one; Don't need to map nodes or elements...
  return true;
}

namespace {
template <typename INT>
void
Compute_Node_Map(INT *& node_map, ExoII_Read<INT> & file1, ExoII_Read<INT> & file2)
{
  // This function is called if and only if there are nodes that were
  // not matched in the Compute_Map function.  This is typically the
  // case if there are 'free' nodes which are not connected to any
  // elements.

  size_t num_nodes = file1.Num_Nodes();
  auto mapped_2 = new INT[num_nodes];

  // Cannot ignore the comparisons, so make sure the coord_tol_type
  // is not -1 which is "ignore"
  TOLERANCE_TYPE_enum save_tolerance_type = interface.coord_tol.type;
  if (save_tolerance_type == IGNORE)
  {
    interface.coord_tol.type = ABSOLUTE;
  }

  // Initialize...
  for (size_t i = 0; i < num_nodes; i++)
  {
    mapped_2[i] = -1;
  }

  // Find unmapped nodes in file2; count the unmapped nodes in file_1.
  // The code below sets the 'mapped_2' entry to 1 for each node in
  // file2 which has been mapped to a node in file1
  size_t count_1 = 0;
  for (size_t i = 0; i < num_nodes; i++)
  {
    if (node_map[i] != -1)
    {
      mapped_2[node_map[i]] = 1;
    }
    else
    {
      count_1++;
    }
  }

  // Get list of all unmapped nodes in file1 and file2. A file1
  // unmapped node will have a '-1' entry in 'node_map' and a file2
  // unmapped node will have a '-1' entry in 'mapped_2'.  Reuse the
  // 'mapped_2' array to hold the list.
  auto mapped_1 = new INT[count_1];
  size_t count_2 = 0;
  count_1 = 0;
  for (size_t i = 0; i < num_nodes; i++)
  {
    if (node_map[i] == -1)
    {
      mapped_1[count_1++] = i;
    }
    if (mapped_2[i] == -1)
    {
      mapped_2[count_2++] = i;
    }
  }

  // check that umnapped node counts are equal.  If not, output
  // message and exit.
  if (count_1 != count_2)
  {
    ERROR("Files are different (free node count in file1 is "
          << count_1 << " but file2 free node count is " << count_2 << ")\n");
    exit(1);
  }

  // Now, need to match all nodes in 'mapped_1' with nodes in
  // 'mapped_2'
  // Get pointer to coordinates...
  const double * x1_f = (double *)file1.X_Coords();
  const double * y1_f = (double *)file1.Y_Coords();
  const double * z1_f = (double *)file1.Z_Coords();

  const double * x2_f = (double *)file2.X_Coords();
  const double * y2_f = (double *)file2.Y_Coords();
  const double * z2_f = (double *)file2.Z_Coords();

  // For now, we will try a brute force matching with the hopes that
  // the number of unmatched nodes is 'small'.  If this proves to be a
  // bottleneck, replace with a better algorithm; perhaps the sorted
  // matching process used in gjoin...
  size_t matched = 0;
  int dim = file1.Dimension();
  size_t j;
  for (size_t i = 0; i < count_1; i++)
  {
    size_t id_1 = mapped_1[i];
    for (j = 0; j < count_2; j++)
    {
      if (mapped_2[j] >= 0)
      {
        size_t id_2 = mapped_2[j];
        if ((dim == 1 && !interface.coord_tol.Diff(x1_f[id_1], x2_f[id_2])) ||
            (dim == 2 && !interface.coord_tol.Diff(x1_f[id_1], x2_f[id_2]) &&
             !interface.coord_tol.Diff(y1_f[id_1], y2_f[id_2])) ||
            (dim == 3 && !interface.coord_tol.Diff(x1_f[id_1], x2_f[id_2]) &&
             !interface.coord_tol.Diff(y1_f[id_1], y2_f[id_2]) &&
             !interface.coord_tol.Diff(z1_f[id_1], z2_f[id_2])))
        {
          node_map[id_1] = id_2;
          mapped_2[j] = -1;
          matched++;
          break;
        }
      }
    }
  }

  // Check that all nodes were matched.
  if (matched != count_1)
  {
    ERROR("Unable to match all free nodes in the model.  There are "
          << count_1 - matched << " unmatched nodes remaining.\n");
    exit(1);
  }

  // Free memory and return.
  delete[] mapped_1;
  delete[] mapped_2;

  interface.coord_tol.type = save_tolerance_type;
  }

  template <typename INT>
  INT
  Find(double x0,
       double y0,
       double z0,
       double * x,
       double * y,
       double * z,
       INT * id,
       size_t N,
       int dim,
       bool ignore_dups)
  {
    SMART_ASSERT(x != nullptr);
    SMART_ASSERT(N > 0);

    // Cannot ignore the comparisons, so make sure the coord_tol_type
    // is not -1 which is "ignore"
    TOLERANCE_TYPE_enum save_tolerance_type = interface.coord_tol.type;
    if (save_tolerance_type == IGNORE)
    {
      interface.coord_tol.type = ABSOLUTE;
    }

    // Find the index such that x0 > x[0,1,...,low-1] and x0 >= x[low]
    // where x[N] is infinity.

    size_t mid, low = 0, high = N;
    while (low < high)
    {
      mid = (low + high) / 2;
      if (x[id[mid]] < x0)
      {
        low = mid + 1;
      }
      else
      {
        high = mid;
      }
    }

    INT i = low == N ? N - 1 : low; // Make sure index falls within array bounds.

    // Drop to first index before which the tolerance fails.
    while (i > 0 && !interface.coord_tol.Diff(x[id[i - 1]], x0))
    {
      --i;
    }

    // Search until tolerance between the x coordinate fails or a match is found.
    // If a match is found, the loop continues in order to check for dups.

    INT index = -1;
    do {
      if (dim == 1 || (dim == 2 && !interface.coord_tol.Diff(y[id[i]], y0)) ||
          (dim == 3 && !interface.coord_tol.Diff(y[id[i]], y0) &&
           !interface.coord_tol.Diff(z[id[i]], z0)))
      {
        if (index >= 0)
        {
          if (ignore_dups)
          {
            return index;
          }

          double x1 = x[id[i]];
          double y1 = dim > 1 ? y[id[i]] : 0.0;
          double z1 = dim > 2 ? z[id[i]] : 0.0;

          double x2 = x[id[index]];
          double y2 = dim > 1 ? y[id[index]] : 0.0;
          double z2 = dim > 2 ? z[id[index]] : 0.0;

          ERROR("Two elements in file 2 have the "
                << "same midpoint (within tolerance).\n"
                << "\tLocal element  " << id[i] + 1 << " at (" << x1 << ", " << y1 << ", " << z1
                << ") and\n"
                << "\tLocal element " << id[index] + 1 << " at (" << x2 << ", " << y2 << ", " << z2
                << ")\n"
                << "\tNo unique element mapping possible.\n"
                << '\n');
          return -1;
        }

        index = i;
      }
    } while (++i < (INT)N && !interface.coord_tol.Diff(x[id[i]], x0));

    interface.coord_tol.type = save_tolerance_type;
    return index;
  }

  inline double
  dist_sqrd(double x1, double x2)
  {
    return (x2 - x1) * (x2 - x1);
  }

  inline double dist_sqrd(double x1, double y1, double x2, double y2)
  {
    double d1 = x2 - x1;
    d1 *= d1;
    double d2 = y2 - y1;
    d2 *= d2;
    return (d1 + d2);
  }

  inline double
  dist_sqrd(double x1, double y1, double z1, double x2, double y2, double z2)
  {
    double d1 = x2 - x1;
    d1 *= d1;
    double d2 = y2 - y1;
    d2 *= d2;
    double d3 = z2 - z1;
    d3 *= d3;
    return (d1 + d2 + d3);
  }

  double find_range(const double *x, size_t num_nodes)
  {
    double rmin = x[0];
    double rmax = x[0];
    for (size_t i = 1; i < num_nodes; i++)
    {
      rmin = rmin < x[i] ? rmin : x[i];
      rmax = rmax > x[i] ? rmax : x[i];
    }
    return rmax - rmin;
  }
  } // namespace

  template <typename INT>
  double
  Find_Min_Coord_Sep(ExoII_Read<INT> & file)
  {
    size_t num_nodes = file.Num_Nodes();
    if (num_nodes < 2)
    {
      return 0.0;
    }

    file.Load_Nodal_Coordinates();
    const double * x = (double *)file.X_Coords();
    const double * y = (double *)file.Y_Coords();
    const double * z = (double *)file.Z_Coords();

    auto indx = new INT[num_nodes];
    for (size_t i = 0; i < num_nodes; i++)
    {
      indx[i] = i;
    }

    // Find coordinate with largest range...
    const double * r = x;
    double range = find_range(x, num_nodes);
    if (file.Dimension() > 1)
    {
      double yrange = find_range(y, num_nodes);
      if (yrange > range)
      {
        range = yrange;
        r = y;
      }
    }

    if (file.Dimension() > 2)
    {
      double zrange = find_range(z, num_nodes);
      if (zrange > range)
      {
        range = zrange;
        r = z;
      }
    }

    // Sort based on coordinate with largest range...
    index_qsort(r, indx, num_nodes);

    double min = DBL_MAX;
    ;
    switch (file.Dimension())
    {
      case 1:
      {
        for (size_t i = 0; i < num_nodes; i++)
        {
          for (size_t j = i + 1; j < num_nodes; j++)
          {
            double tmp = dist_sqrd(x[indx[i]], x[indx[j]]);
            if (tmp >= min)
            {
              break;
            }
            else
            {
              min = tmp;
            }
          }
        }
        break;
      }
      case 2:
      {
        for (size_t i = 0; i < num_nodes; i++)
        {
          for (size_t j = i + 1; j < num_nodes; j++)
          {
            double delr = dist_sqrd(r[indx[i]], r[indx[j]]);
            if (delr > min)
            {
              break;
            }
            else
            {
              double tmp = dist_sqrd(x[indx[i]], y[indx[i]], x[indx[j]], y[indx[j]]);
              min = min < tmp ? min : tmp;
            }
          }
        }
        break;
      }
      case 3:
      {
        for (size_t i = 0; i < num_nodes; i++)
        {
          for (size_t j = i + 1; j < num_nodes; j++)
          {
            double delr = dist_sqrd(r[indx[i]], r[indx[j]]);
            if (delr > min)
            {
              break;
            }
            else
            {
              double tmp =
                  dist_sqrd(x[indx[i]], y[indx[i]], z[indx[i]], x[indx[j]], y[indx[j]], z[indx[j]]);
              min = min < tmp ? min : tmp;
            }
          }
        }
        break;
      }
    }
    delete[] indx;
    return sqrt(min);
}

template <typename INT>
bool
Compare_Maps(ExoII_Read<INT> & file1,
             ExoII_Read<INT> & file2,
             const INT * node_map,
             const INT * elmt_map,
             bool partial_flag)
{
  // Check whether the node and element number maps from both file1
  // and file2 match which indicates that we are comparing the same
  // element and node in each file.

  size_t num_nodes1 = file1.Num_Nodes();
  size_t num_elmts1 = file1.Num_Elmts();

  // size_t num_nodes2 = file2.Num_Nodes();
  // size_t num_elmts2 = file2.Num_Elmts();

  // NOTE: file1 maps are already loaded...
  file2.Load_Node_Map();
  file2.Load_Elmt_Map();

  const INT *node_id_map1 = file1.Get_Node_Map();
  const INT *elem_id_map1 = file1.Get_Elmt_Map();

  const INT *node_id_map2 = file2.Get_Node_Map();
  const INT *elem_id_map2 = file2.Get_Elmt_Map();

  bool diff = false;
  size_t warn_count = 0;

  if (node_map != nullptr)
  {
    if (!interface.dump_mapping)
    {
      // There is a map between file1 and file2, but all nodes are
      // used in both files.
      for (size_t i = 0; i < num_nodes1; i++)
      {
        if (node_id_map1[i] != node_id_map2[node_map[i]])
        {
          if (!(node_id_map2[node_map[i]] == 0 && partial_flag))
          { // Don't output diff if non-matched and partial
            std::cerr << "exodiff: WARNING .. The local node " << i + 1 << " with global id "
                      << node_id_map1[i] << " in file1 has the global id "
                      << node_id_map2[node_map[i]] << " in file2.\n";
            diff = true;
            warn_count++;
            if (warn_count > 100)
            {
              std::cerr << "exodiff: WARNING .. Too many warnings, skipping remainder...\n";
              break;
            }
          }
        }
      }
    }
  }
  else
  {
    // No node mapping between file1 and file2 -- do a straight compare.
    for (size_t i = 0; i < num_nodes1; i++)
    {
      if (node_id_map1[i] != node_id_map2[i]) {
        if (!(node_id_map2[i] == 0 && partial_flag))
        { // Don't output diff if non-matched and partial
          std::cerr << "exodiff: WARNING .. The local node " << i + 1 << " with global id "
                    << node_id_map1[i] << " in file1 has the global id " << node_id_map2[i]
                    << " in file2.\n";
          diff = true;
          warn_count++;
          if (warn_count > 100)
          {
            std::cerr << "exodiff: WARNING .. Too many warnings, skipping remainder...\n";
            break;
          }
        }
      }
    }
  }

  warn_count = 0;
  if (elmt_map != nullptr)
  {
    if (!interface.dump_mapping)
    {
      // There is a map between file1 and file2, but all elements are
      // used in both files.
      for (size_t i = 0; i < num_elmts1; i++)
      {
        if (elem_id_map1[i] != elem_id_map2[elmt_map[i]])
        {
          if (!(elem_id_map2[elmt_map[i]] == 0 && partial_flag))
          { // Don't output diff if non-matched and partial
            std::cerr << "exodiff: WARNING .. The local element " << i + 1 << " with global id "
                      << elem_id_map1[i] << " in file1 has the global id "
                      << elem_id_map2[elmt_map[i]] << " in file2.\n";
            diff = true;
            warn_count++;
            if (warn_count > 100)
            {
              std::cerr << "exodiff: WARNING .. Too many warnings, skipping remainder...\n";
              break;
            }
          }
        }
      }
    }
  }
  else
  {
    // No element mapping between file1 and file2 -- do a straight compare.
    for (size_t i = 0; i < num_elmts1; i++)
    {
      if (elem_id_map1[i] != elem_id_map2[i]) {
        if (!(elem_id_map2[i] == 0 && partial_flag))
        { // Don't output diff if non-matched and partial
          std::cerr << "exodiff: WARNING .. The local element " << i + 1 << " with global id "
                    << elem_id_map1[i] << " in file1 has the global id " << elem_id_map2[i]
                    << " in file2.\n";
          diff = true;
          warn_count++;
          if (warn_count > 100)
          {
            std::cerr << "exodiff: WARNING .. Too many warnings, skipping remainder...\n";
            break;
          }
        }
      }
    }
  }
  file2.Free_Node_Map();
  file2.Free_Elmt_Map();

  if (diff)
  {
    std::cout << '\n';
  }
  return diff;
}

template void
Compute_Maps(int *& node_map, int *& elmt_map, ExoII_Read<int> & file1, ExoII_Read<int> & file2);
template bool Compare_Maps(ExoII_Read<int> & file1,
                           ExoII_Read<int> & file2,
                           const int * node_map,
                           const int * elmt_map,
                           bool partial_flag);

template void Compute_Partial_Maps(int *& node_map,
                                   int *& elmt_map,
                                   ExoII_Read<int> & file1,
                                   ExoII_Read<int> & file2);
template void Compute_FileId_Maps(int *& node_map,
                                  int *& elmt_map,
                                  ExoII_Read<int> & file1,
                                  ExoII_Read<int> & file2);
template void Dump_Maps(const int * node_map, const int * elmt_map, ExoII_Read<int> & file1);
template bool Check_Maps(const int * node_map,
                         const int * elmt_map,
                         const ExoII_Read<int> & file1,
                         const ExoII_Read<int> & file2);
template double Find_Min_Coord_Sep(ExoII_Read<int> & file);

template void Compute_Maps(int64_t *& node_map,
                           int64_t *& elmt_map,
                           ExoII_Read<int64_t> & file1,
                           ExoII_Read<int64_t> & file2);
template bool Compare_Maps(ExoII_Read<int64_t> & file1,
                           ExoII_Read<int64_t> & file2,
                           const int64_t * node_map,
                           const int64_t * elmt_map,
                           bool partial_flag);

template void Compute_Partial_Maps(int64_t *& node_map,
                                   int64_t *& elmt_map,
                                   ExoII_Read<int64_t> & file1,
                                   ExoII_Read<int64_t> & file2);
template void Compute_FileId_Maps(int64_t *& node_map,
                                  int64_t *& elmt_map,
                                  ExoII_Read<int64_t> & file1,
                                  ExoII_Read<int64_t> & file2);
template void
Dump_Maps(const int64_t * node_map, const int64_t * elmt_map, ExoII_Read<int64_t> & file1);
template bool Check_Maps(const int64_t * node_map,
                         const int64_t * elmt_map,
                         const ExoII_Read<int64_t> & file1,
                         const ExoII_Read<int64_t> & file2);
template double Find_Min_Coord_Sep(ExoII_Read<int64_t> & file);
