// Copyright(C) 2008 Sandia Corporation.  Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
// certain rights in this software
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
//     * Neither the name of Sandia Corporation nor the names of its
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

#include <cstdlib>
#include <cstdio>
#include <math.h>
#include <ctype.h>
#include <vector>

#include "Tolerance.h"
#include "smart_assert.h"
#include "exoII_read.h"
#include "exo_block.h"
#include "node_set.h"
#include "side_set.h"
#include "exodusII.h"
#include "stringx.h"
#include "Specifications.h"

using namespace std;

namespace {
  bool Check_Nodal(ExoII_Read& file1, ExoII_Read& file2, bool check_only);
  bool Check_Elmt_Block(ExoII_Read& file1, ExoII_Read& file2, bool check_only);
  bool Check_Nodeset(ExoII_Read& file1, ExoII_Read& file2, const int *node_map, bool check_only);
  bool Check_Sideset(ExoII_Read& file1, ExoII_Read& file2, const int *elmt_map, bool check_only);

  bool Check_Elmt_Block_Params(const Exo_Block*, const Exo_Block*);
  bool Check_Elmt_Block_Connectivity(Exo_Block*, Exo_Block*);
  bool close_compare(const string & st1, const string & st2);
}

bool Check_Global(ExoII_Read& file1, ExoII_Read& file2)
{
  bool is_same = true;
  if (file1.Dimension() != file2.Dimension()) {
    std::cout << "exodiff: ERROR .. Dimension doesn't agree." << std::endl;
    is_same = false;
  }
  if (file1.Num_Nodes() != file2.Num_Nodes()) {
    if(specs.map_flag != PARTIAL){
      std::cout << "exodiff: ERROR .. Number of nodes don't agree." << std::endl;
      is_same = false;
    }
  }
  if (file1.Num_Elmts() != file2.Num_Elmts()) {
    if(specs.map_flag != PARTIAL){
      std::cout << "exodiff: ERROR .. Number of elements don't agree." << std::endl;
      is_same = false;
    }
  }
  if (!specs.map_flag && file1.Num_Elmt_Blocks() != file2.Num_Elmt_Blocks()) {
    if(specs.map_flag != PARTIAL){
      std::cout << "exodiff: ERROR .. Number of blocks don't agree." << std::endl;
      is_same = false;
    }
  }
  if (!specs.map_flag && file1.Num_Times() != file2.Num_Times() && !specs.quiet_flag) {
    std::cout << "exodiff: WARNING First file has " << file1.Num_Times()
	      << " result times while the second file has " << file2.Num_Times()
	      << ".\n"
	      << "         Will consider only "
	      << (file1.Num_Times() < file2.Num_Times() ? file1.Num_Times()
		  : file2.Num_Times())
	      << " timesteps." << std::endl;
  }
  return is_same;
}

void Check_Compatible_Meshes(ExoII_Read& file1, ExoII_Read& file2, bool check_only,
			     int *node_map, int *elmt_map)
{
  bool is_diff = false;
  if (!Check_Global(file1, file2))
    is_diff = true;
  
  if (!specs.map_flag) {
    if (!Check_Nodal(file1, file2, check_only))
      is_diff = true;
    if (!Check_Elmt_Block(file1, file2, check_only))
      is_diff = true;
  }
  if (!Check_Nodeset(file1, file2, node_map, check_only))
    is_diff = true;

  if (!Check_Sideset(file1, file2, elmt_map, check_only))
    is_diff = true;

  if (is_diff) {
    std::cout << "exodiff: ERROR .. Differences found in mesh metadata.  Aborting...\n";
    exit(1);
  }
}

namespace {
  bool Check_Nodal(ExoII_Read& file1, ExoII_Read& file2, bool check_only)
  {
    bool is_same = true;
  
    file1.Load_Nodal_Coordinates();
    file2.Load_Nodal_Coordinates();

    double* x1 = (double*)file1.X_Coords();
    double *y1 = x1, *z1 = x1;
    if (file1.Dimension() > 1) y1 = (double*)file1.Y_Coords();
    if (file1.Dimension() > 2) z1 = (double*)file1.Z_Coords();
  
    double* x2 = (double*)file2.X_Coords();
    double *y2 = x2, *z2 = x2;
    if (file2.Dimension() > 1) y2 = (double*)file2.Y_Coords();
    if (file2.Dimension() > 2) z2 = (double*)file2.Z_Coords();
  
    double max = 0.0, norm;
    for (int n = 0; n < file1.Num_Nodes() && is_same; ++n) {
      if (check_only) {
	if (specs.coord_tol.Diff(x1[n], x2[n])) {
	  std::cout << "exodiff: ERROR .. Files are different (x nodal coordinate "
		    << (n+1) << ")" << std::endl;
	  is_same = false;
	}
      }
      norm = (x1[n] - x2[n])*(x1[n] - x2[n]);
      if (file1.Dimension() > 1) {
	if (check_only) {
	  if (specs.coord_tol.Diff(y1[n], y2[n])) {
	    std::cout << "exodiff: ERROR .. Files are different (y nodal coordinate "
		      << (n+1) << ")" << std::endl;
	    is_same = false;
	  }
	}
	norm += (y1[n] - y2[n])*(y1[n] - y2[n]);
      }
      if (file1.Dimension() > 2) {
	if (check_only) {
	  if (specs.coord_tol.Diff(z1[n], z2[n])) {
	    std::cout << "exodiff: ERROR .. Files are different (z nodal coordinate "
		      << (n+1) << ")" << std::endl;
	    is_same = false;
	  }
	}
	norm += (z1[n] - z2[n])*(z1[n] - z2[n]);
      }
      max = max < norm ? norm: max;
    }
  
    if (!specs.quiet_flag && !check_only) {
      max = sqrt(max);
      std::cout << "Maximum difference between Genesis nodal coordinates = "
		<< max << std::endl;
    }
    file1.Free_Nodal_Coordinates();
    file2.Free_Nodal_Coordinates();
    return is_same;
  }

  bool Check_Elmt_Block(ExoII_Read& file1, ExoII_Read& file2, bool check_only)
  {
    bool is_same = true;
    // Verify that element blocks match in the two files...
    for (int b = 0; b < file1.Num_Elmt_Blocks(); ++b) {
      Exo_Block* block1 = file1.Get_Elmt_Block_by_Index(b);
      Exo_Block* block2 = file2.Get_Elmt_Block_by_Index(b);
      if (specs.map_flag != DISTANCE && specs.map_flag != PARTIAL) {
	if (block1->Id() != block2->Id()) {
	  block2 = file2.Get_Elmt_Block_by_Id(block1->Id());
	  if (block2 == NULL) {
	    std::cout << "exodiff: ERROR .. Block id " << block1->Id()
		      << " exists in first "
		      << "file but not the second." << std::endl;
	    is_same = false;
	  }
	}
	if (block1 != NULL && block2 != NULL) {
	  if (!Check_Elmt_Block_Params(block1, block2)) {
	    is_same = false;
	  } else {
	    // Only do this check if Check_Elmt_Block_Params does not fail.
	    if (!Check_Elmt_Block_Connectivity(block1, block2))
	      is_same = false;
	  }
	}
      }
    }
    return is_same;
  }

  bool Check_Elmt_Block_Connectivity(Exo_Block* block1, Exo_Block* block2)
  {
    bool is_same = true;
    SMART_ASSERT(block1 && block2);
  
    block1->Load_Connectivity();
    block2->Load_Connectivity();
    const int* conn1 = block1->Connectivity();
    const int* conn2 = block2->Connectivity();

    SMART_ASSERT( block1->Size() == 0 ||
		  block1->Num_Nodes_per_Elmt() == 0 ||
		  conn1 != NULL );
    SMART_ASSERT( block2->Size() == 0 ||
		  block2->Num_Nodes_per_Elmt() == 0 ||
		  conn2 != NULL );
  
    int node_count =  block1->Size() * block1->Num_Nodes_per_Elmt();
    SMART_ASSERT(node_count == block2->Size() * block2->Num_Nodes_per_Elmt());
  
    for (int e = 0; e < node_count; ++e) {
      if (conn1[e] != conn2[e]) {
	std::cout << "exodiff: ERROR .. Connectivities for block id " << block1->Id()
		  << " are not the same." << std::endl;
	is_same = false;
	break;
      }
    }
    block2->Free_Connectivity();
    block1->Free_Connectivity();
    return is_same;
  }

  bool Check_Elmt_Block_Params(const Exo_Block* block1, const Exo_Block* block2)
  {
    bool is_same = true;
    SMART_ASSERT(block1 && block2);
  
    if (block1->Id() != block2->Id()) {
      std::cout << "exodiff: ERROR .. Block ids don't agree (" << block1->Id()
		<< " != " << block2->Id() << ")." << std::endl;
      is_same = false;
    }
    if (!( no_case_equals( block1->Elmt_Type(), block2->Elmt_Type() ) ) ) {
      if ( !specs.short_block_check ||
	   !close_compare(block1->Elmt_Type(), block2->Elmt_Type()) ) {
	std::cout << "exodiff: ERROR .. Block element types don't agree ("
		  << block1->Elmt_Type()
		  << " != " << block2->Elmt_Type() << ")." << std::endl;
	is_same = false;
      }
    }
    if (block1->Size() != block2->Size()) {
      std::cout << "exodiff: ERROR .. Block number of elements don't agree ("
		<< block1->Size()
		<< " != " << block2->Size() << ")."
		<< std::endl;
      is_same = false;
    }
    if (block1->Num_Nodes_per_Elmt() != block2->Num_Nodes_per_Elmt()) {
      std::cout << "exodiff: ERROR .. Block number of nodes per element don't agree ("
		<< block1->Num_Nodes_per_Elmt()
		<< " != " << block2->Num_Nodes_per_Elmt() << ")."
		<< std::endl;
      is_same = false;
    }
#if 0
    if (block1->Num_Attributes() != block2->Num_Attributes()) {
      std::cout << "exodiff: ERROR .. Block number of attributes don't agree ("
		<< block1->Num_Attributes()
		<< " != " << block2->Num_Attributes() << ")."
		<< std::endl;
      is_same = false;
    }
#endif
    return is_same;
  }

  bool Check_Nodeset(ExoII_Read& file1, ExoII_Read& file2, const int *node_map, bool)
  {
    // Currently don't set diff flag for most of these since we
    // can continue (somewhat) with these differences...
    // As get more usage of nodeset/sideset variables, may revisit
    // what is a diff.
    bool is_same = true;
    if (file1.Num_Node_Sets() != file2.Num_Node_Sets()) {
      if(specs.map_flag != PARTIAL){
	std::cout << "exodiff: WARNING .. Number of nodesets don't agree...\n";
      }
    }
    // Check that the files both contain the same nodesets...
    for (int b = 0; b < file1.Num_Node_Sets(); ++b) {
      Node_Set* set1 = file1.Get_Node_Set_by_Index(b);
      Node_Set* set2 = file2.Get_Node_Set_by_Id(set1->Id());
      if (set2 == NULL) {
	std::cout << "exodiff: WARNING .. Nodeset id " << set1->Id()
		  << " exists in first file but not the second.\n";
      } else {
	if (set1->Size() != set2->Size()) {
	  std::cout << "exodiff: WARNING .. The node count for nodeset id " << set1->Id()
		    << " is not the same in the two files.\n";
	}
      }
    }

    for (int b = 0; b < file2.Num_Node_Sets(); ++b) {
      Node_Set* set2 = file2.Get_Node_Set_by_Index(b);
      if (set2 == NULL) {
	std::cout << "exodiff: WARNING .. Nodeset id " << set2->Id()
		  << " exists in second file but not the first.\n";
      }
    }


    // Do the following check(s) only if there are nodeset varibles...
    // For each nodeset, check that the order of the nodeset nodes is the same.
    // Eventually need to be able to map the order...
    if (specs.ns_var_names->size() > 0) {
      for (int b = 0; b < file1.Num_Node_Sets(); ++b) {
	Node_Set* set1 = file1.Get_Node_Set_by_Index(b);
	Node_Set* set2 = file2.Get_Node_Set_by_Id(set1->Id());
	if (set2 == NULL)
	  continue;

	if (node_map != NULL)
	  set1->apply_map(node_map);
	
	if (set1->var_count() > 0 && (set1->Size() == set2->Size())) {
	  int node_count = set1->Size();
	  int diff = -1;
	  for (int i=0; i < node_count; i++) {
	    if (set1->Node_Id(i) != set2->Node_Id(i)) {
	      diff = i;
	      break;
	    }
	  }
	  if (diff >= 0) {
	    std::cout << "exodiff: ERROR .. The nodelists for nodeset id " << set1->Id()
		      << " are not the same in the two files.\n"
		      << "\tThe first difference is at position " << set1->Node_Index(diff)+1
		      << ": Node " << set1->Node_Id(diff) << " vs. Node " << set2->Node_Id(diff) <<".\n";
	    if(specs.map_flag != PARTIAL){
	      is_same = false;
	    } else {
	      std::cout << "exodiff: ERROR .. The nodelist differences are ignored for the partial_map case.\n";
	    }
	  }
	}
      }
    }
    return is_same;
  }
 
  bool Check_Sideset(ExoII_Read& file1, ExoII_Read& file2, const int *elmt_map, bool)
  {
    // Currently don't set diff flag for most of these since we
    // can continue (somewhat) with these differences...
    // As get more usage of nodeset/sideset variables, may revisit
    // what is a diff.
    bool is_same = true;
    if (file1.Num_Side_Sets() != file2.Num_Side_Sets()) {
      if(specs.map_flag != PARTIAL){
	std::cout << "exodiff: WARNING .. Number of sidesets don't agree...\n";
      }
    }
    // Check that the files both contain the same sidesets...
    for (int b = 0; b < file1.Num_Side_Sets(); ++b) {
      Side_Set* set1 = file1.Get_Side_Set_by_Index(b);
      Side_Set* set2 = file2.Get_Side_Set_by_Id(set1->Id());
      if (set2 == NULL) {
	std::cout << "exodiff: WARNING .. Sideset id " << set1->Id()
		  << " exists in first file but not the second.\n";
      } else {
	if (set1->Size() != set2->Size()) {
	  std::cout << "exodiff: WARNING .. The side count for sideset id " << set1->Id()
		    << " is not the same in the two files.\n";
	}
      }
    }

    for (int b = 0; b < file2.Num_Side_Sets(); ++b) {
      Side_Set* set2 = file2.Get_Side_Set_by_Index(b);
      if (set2 == NULL) {
	std::cout << "exodiff: WARNING .. Sideset id " << set2->Id()
		  << " exists in second file but not the first.\n";
      }
    }


    // Do the following check(s) only if there are sideset varibles...
    // For each sideset, check that the order of the sideset sides is the same.
    // Eventually need to be able to map the order...
    if (specs.ss_var_names->size() > 0) {
      for (int b = 0; b < file1.Num_Side_Sets(); ++b) {
	Side_Set* set1 = file1.Get_Side_Set_by_Index(b);
	Side_Set* set2 = file2.Get_Side_Set_by_Id(set1->Id());
	if (set2 == NULL)
	  continue;

	if (elmt_map != NULL)
	  set1->apply_map(elmt_map);
      
	// Don't care if sidesets don't match if there are no variables...
	if (set1->var_count() > 0 && (set1->Size() == set2->Size())) {
	  int side_count = set1->Size();
	  int diff = -1;
	  for (int i=0; i < side_count; i++) {
	    if (set1->Side_Id(i) != set2->Side_Id(i)) {
	      diff = i;
	      break;
	    }
	  }
	  if (diff >= 0) {
	    std::cout << "exodiff: ERROR .. The sidelists for sideset id " << set1->Id()
		      << " are not the same in the two files.\n"
		      << "\tThe first difference is at position " << set1->Side_Index(diff)+1
		      << ": Side "
		      << set1->Side_Id(diff).first << "." << set1->Side_Id(diff).second
		      << " .vs. Side "
		      << set2->Side_Id(diff).first << "." << set2->Side_Id(diff).second
		      << ".\n";
	    if(specs.map_flag != PARTIAL){
	      is_same = false;
	    } else {
	      std::cout << "exodiff: ERROR .. The sidelist differences are ignored for the partial_map case.\n";
	    }
	  }
	}
      }
    }
    return is_same;
  }
 
  bool close_compare(const string & st1, const string & st2)
  {
    unsigned len1 = st1.size();
    unsigned len2 = st2.size();
  
    // Check that digits (if any) at end of names match
    while (isdigit(st1[len1-1]) && isdigit(st2[len2-1])) {
      if (st1[len1-1] != st2[len2-1])
	return false;
      len1--; len2--;
    }

    // Skip any digits at the end.  It's OK if only one name has
    // digits since we want 'tri' and 'triangle6' to match, but not
    // tri6 and tri3 nor quad9 and tri9
    while (isdigit(st1[len1-1])) {
      len1--;
    }
  
    while (isdigit(st2[len2-1])) {
      len2--;
    }
  
    unsigned length = (len1 < len2) ? len1 : len2;
    for (unsigned i = 0; i < length; ++i) {
      if (toupper(st1[i]) != toupper(st2[i]))
	return false;
    }
    return true;
  }


}
