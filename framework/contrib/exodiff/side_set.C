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
#include "libmesh/exodusII.h"
#include "ED_SystemInterface.h"

using namespace std;

template <typename INT>
Side_Set<INT>::Side_Set()
  : Exo_Entity(),
    num_dist_factors(0),
    elmts(0),
    sides(0),
    sideIndex(0),
    dist_factors(0)
{ }

template <typename INT>
Side_Set<INT>::Side_Set(int file_id, size_t id)
  : Exo_Entity(file_id, id),
    num_dist_factors(0),
    elmts(0),
    sides(0),
    sideIndex(0),
    dist_factors(0)
{
  SMART_ASSERT((int)id != EX_INVALID_ID);
}

template <typename INT>
Side_Set<INT>::Side_Set(int file_id, size_t id, size_t ns, size_t ndf)
  : Exo_Entity(file_id, id, ns),
    num_dist_factors(ndf),
    elmts(0),
    sides(0),
    sideIndex(0),
    dist_factors(0)
{
  SMART_ASSERT(id > 0);
}

template <typename INT>
Side_Set<INT>::~Side_Set()
{
  SMART_ASSERT(Check_State());

  delete [] elmts;
  delete [] sides;
  delete [] sideIndex;
  delete [] dist_factors;
}

template <typename INT>
EXOTYPE Side_Set<INT>::exodus_type() const {return EX_SIDE_SET;}

template <typename INT>
void Side_Set<INT>::entity_load_params()
{
  int err = ex_get_set_param(fileId, EX_SIDE_SET, id_, &numEntity,&num_dist_factors);

  if (err < 0) {
    std::cout << "ERROR: Failed to get sideset parameters for sideset " << id_
	      << ". !  Aborting..." << std::endl;
    exit(1);
  }
}

template <typename INT>
void Side_Set<INT>::apply_map(const INT *elmt_map)
{
  SMART_ASSERT(elmt_map != NULL);
  if (elmts != NULL) {
    delete [] elmts;     elmts = NULL;
    delete [] sides;     sides = NULL;
    delete [] sideIndex; sideIndex = NULL;
  }
  load_sides(elmt_map);
}

template <typename INT>
void Side_Set<INT>::load_sides(const INT *elmt_map) const
{
  int err = 0;
  if ((elmts == NULL || sides == NULL) && numEntity > 0)
    {
      elmts = new INT[numEntity];  SMART_ASSERT(elmts != 0);
      sides = new INT[numEntity];  SMART_ASSERT(sides != 0);
      sideIndex = new INT[numEntity]; SMART_ASSERT(sideIndex != 0);

      err = ex_get_set(fileId, EX_SIDE_SET, id_, elmts, sides);

      if (err < 0) {
	std::cout << "Side_Set<INT>::Load_Set(): ERROR: Failed to read side set "
		  << id_ << "!  Aborting..." << std::endl;
	exit(1);
      }

      if (elmt_map != NULL) {
	for (size_t i=0; i < numEntity; i++) {
	  elmts[i] = 1+elmt_map[elmts[i]-1];
	}
      }

      if (interface.ssmap_flag) {
	for (size_t i=0; i < numEntity; i++) {
	  sideIndex[i] = i;
	  elmts[i] = elmts[i] * 8 + sides[i];
	}

	index_qsort(elmts, sideIndex, numEntity);

	// Recover elmts...
	for (size_t i=0; i < numEntity; i++) {
	  elmts[i] = elmts[i] / 8;
	}
      } else {
	for (size_t i=0; i < numEntity; i++) {
	  sideIndex[i] = i;
	}
      }
      SMART_ASSERT(Check_State());
    }
}

template <typename INT>
const INT* Side_Set<INT>::Elements() const
{
  load_sides();
  return elmts;
}

template <typename INT>
const INT* Side_Set<INT>::Sides() const
{
  load_sides();
  return sides;
}

template <typename INT>
std::pair<INT,INT> Side_Set<INT>::Side_Id(size_t position) const
{
  load_sides();
  SMART_ASSERT(position < numEntity);
  return std::make_pair(elmts[sideIndex[position]], sides[sideIndex[position]]);
}

template <typename INT>
size_t Side_Set<INT>::Side_Index(size_t position) const
{
  load_sides();
  SMART_ASSERT(position < numEntity);
  return sideIndex[position];
}

template <typename INT>
const double* Side_Set<INT>::Distribution_Factors() const
{
  if (!dist_factors && num_dist_factors > 0) {
    dist_factors = new double[num_dist_factors];  SMART_ASSERT(dist_factors != 0);
    ex_get_set_dist_fact(fileId, EX_SIDE_SET, id_, dist_factors);
  }
  return dist_factors;
}

template <typename INT>
void Side_Set<INT>::Display(std::ostream& s) const
{
  SMART_ASSERT(Check_State());

  s << "Side_Set<INT>::Display()  Exodus side set ID = " << id_              << std::endl
    << "                        number of sides = " << numEntity        << std::endl
    << "         number of distribution factors = " << num_dist_factors << std::endl
    << "                    number of variables = " << var_count()      << std::endl;
}

template <typename INT>
int Side_Set<INT>::Check_State() const
{
  SMART_ASSERT(id_ >= EX_INVALID_ID);
  SMART_ASSERT( !( id_ == EX_INVALID_ID && numEntity > 0 ) );
  SMART_ASSERT( !( id_ == EX_INVALID_ID && num_dist_factors > 0 ) );
  SMART_ASSERT( !( id_ == EX_INVALID_ID && elmts ) );
  SMART_ASSERT( !( id_ == EX_INVALID_ID && sides ) );
  SMART_ASSERT( !( id_ == EX_INVALID_ID && dist_factors ) );

  SMART_ASSERT( !(  elmts && !sides ) );
  SMART_ASSERT( !( !elmts &&  sides ) );

  return 1;
}

template class Side_Set<int>;
template class Side_Set<int64_t>;
