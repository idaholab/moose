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

#include <iostream>
#include <cstdlib>

#include "smart_assert.h"
#include "side_set.h"
#include "iqsort.h"
#include "exodusII.h"
#include "Specifications.h"

using namespace std;

Side_Set::Side_Set()
  : Exo_Entity(),
    num_dist_factors(0),
    elmts(0),
    sides(0),
    sideIndex(0),
    dist_factors(0)
{ }

Side_Set::Side_Set(int file_id, int id)
  : Exo_Entity(file_id, id),
    num_dist_factors(0),
    elmts(0),
    sides(0),
    sideIndex(0),
    dist_factors(0)
{
  SMART_ASSERT(id > 0);
}

Side_Set::Side_Set(int file_id, int id, int ns, int ndf)
  : Exo_Entity(file_id, id, ns),
    num_dist_factors(ndf),
    elmts(0),
    sides(0),
    sideIndex(0),
    dist_factors(0)
{
  SMART_ASSERT(id > 0);
  SMART_ASSERT(ns >= 0);
  SMART_ASSERT(ndf >= 0);
}

Side_Set::~Side_Set()
{
  SMART_ASSERT(Check_State());
  
  delete [] elmts;
  delete [] sides;
  delete [] sideIndex;
  delete [] dist_factors;
}

EXOTYPE Side_Set::exodus_type() const {return EX_SIDE_SET;}

void Side_Set::entity_load_params()
{
  int err = ex_get_set_param(fileId, EX_SIDE_SET, id_, &numEntity,&num_dist_factors);
  
  if (err < 0) {
    std::cout << "ERROR: Failed to get sideset parameters for sideset " << id_
	      << ". !  Aborting..." << std::endl;
    exit(1);
  }
}

void Side_Set::apply_map(const int *elmt_map)
{
  SMART_ASSERT(elmt_map != NULL);
  if (elmts != NULL) {
    delete [] elmts;     elmts = NULL;
    delete [] sides;     sides = NULL;
    delete [] sideIndex; sideIndex = NULL;
  }
  load_sides(elmt_map);
}

void Side_Set::load_sides(const int *elmt_map) const
{
  int err = 0;
  if ((elmts == NULL || sides == NULL) && numEntity > 0)
    {
      elmts = new int[numEntity];  SMART_ASSERT(elmts != 0);
      sides = new int[numEntity];  SMART_ASSERT(sides != 0);
      sideIndex = new int[numEntity]; SMART_ASSERT(sideIndex != 0);

      err = ex_get_set(fileId, EX_SIDE_SET, id_, elmts, sides);
    
      if (err < 0) {
	std::cout << "Side_Set::Load_Set(): ERROR: Failed to read side set "
		  << id_ << "!  Aborting..." << std::endl;
	exit(1);
      }

      if (elmt_map != NULL) {
	for (int i=0; i < numEntity; i++) {
	  elmts[i] = 1+elmt_map[elmts[i]-1];
	}
      }

      if (specs.ssmap_flag) {
	for (int i=0; i < numEntity; i++) {
	  sideIndex[i] = i;
	  elmts[i] = elmts[i] * 8 + sides[i];
	}

	index_qsort(elmts, sideIndex, numEntity);

	// Recover elmts...
	for (int i=0; i < numEntity; i++) {
	  elmts[i] = elmts[i] / 8;
	}
      } else {
	for (int i=0; i < numEntity; i++) {
	  sideIndex[i] = i;
	}
      }
      SMART_ASSERT(Check_State());
    }
}

const int* Side_Set::Elements() const
{
  load_sides();
  return elmts;
}

const int* Side_Set::Sides() const
{
  load_sides();
  return sides;
}

std::pair<int,int> Side_Set::Side_Id(int position) const
{
  load_sides();
  SMART_ASSERT(position < numEntity);
  return std::make_pair(elmts[sideIndex[position]], sides[sideIndex[position]]);
}

int Side_Set::Side_Index(int position) const
{
  load_sides();
  SMART_ASSERT(position < numEntity);
  return sideIndex[position];
}

const double* Side_Set::Distribution_Factors() const
{
  if (!dist_factors && num_dist_factors > 0) {
    dist_factors = new double[num_dist_factors];  SMART_ASSERT(dist_factors != 0);
    ex_get_set_dist_fact(fileId, EX_SIDE_SET, id_, dist_factors);
  }
  return dist_factors;
}

void Side_Set::Display(std::ostream& s) const
{
  SMART_ASSERT(Check_State());
  
  s << "Side_Set::Display()  Exodus side set ID = " << id_              << std::endl
    << "                        number of sides = " << numEntity        << std::endl
    << "         number of distribution factors = " << num_dist_factors << std::endl
    << "                    number of variables = " << var_count()      << std::endl;
}

int Side_Set::Check_State() const
{
  SMART_ASSERT(id_ >= 0);
  SMART_ASSERT(numEntity >= 0);
  SMART_ASSERT(num_dist_factors >= 0);
  
  SMART_ASSERT( !( id_ == 0 && numEntity > 0 ) );
  SMART_ASSERT( !( id_ == 0 && num_dist_factors > 0 ) );
  SMART_ASSERT( !( id_ == 0 && elmts ) );
  SMART_ASSERT( !( id_ == 0 && sides ) );
  SMART_ASSERT( !( id_ == 0 && dist_factors ) );
  
  SMART_ASSERT( !(  elmts && !sides ) );
  SMART_ASSERT( !( !elmts &&  sides ) );
  
  return 1;
}

