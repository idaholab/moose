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

#include "ED_SystemInterface.h" // for ERROR, SystemInterface, etc
#include "libmesh/exodusII.h"   // for ex_set, etc
#include "iqsort.h"             // for index_qsort
#include "side_set.h"
#include "smart_assert.h" // for SMART_ASSERT
#include <cstdlib>        // for exit
#include <iostream>       // for operator<<, basic_ostream, etc
#include <vector>         // for vector

template <typename INT>
Side_Set<INT>::Side_Set()
  : Exo_Entity(),
    num_dist_factors(0),
    elmts(nullptr),
    sides(nullptr),
    sideIndex(nullptr),
    dfIndex(nullptr),
    dist_factors(nullptr)
{
}

template <typename INT>
Side_Set<INT>::Side_Set(int file_id, size_t id)
  : Exo_Entity(file_id, id),
    num_dist_factors(0),
    elmts(nullptr),
    sides(nullptr),
    sideIndex(nullptr),
    dfIndex(nullptr),
    dist_factors(nullptr)
{
  SMART_ASSERT((int)id != EX_INVALID_ID);
}

template <typename INT>
Side_Set<INT>::Side_Set(int file_id, size_t id, size_t ns, size_t ndf)
  : Exo_Entity(file_id, id, ns),
    num_dist_factors(ndf),
    elmts(nullptr),
    sides(nullptr),
    sideIndex(nullptr),
    dfIndex(nullptr),
    dist_factors(nullptr)
{
  SMART_ASSERT(id > 0);
}

template <typename INT>
Side_Set<INT>::~Side_Set()
{
  SMART_ASSERT(Check_State());

  delete[] elmts;
  delete[] sides;
  delete[] sideIndex;
  delete[] dfIndex;
  delete[] dist_factors;
}

template <typename INT>
EXOTYPE
Side_Set<INT>::exodus_type() const
{
  return EX_SIDE_SET;
}

template <typename INT>
void
Side_Set<INT>::entity_load_params()
{
  std::vector<ex_set> sets(1);
  sets[0].id = id_;
  sets[0].type = EX_SIDE_SET;
  sets[0].entry_list = nullptr;
  sets[0].extra_list = nullptr;
  sets[0].distribution_factor_list = nullptr;

  int err = ex_get_sets(fileId, 1, &sets[0]);

  if (err < 0)
  {
    ERROR("Failed to get sideset parameters for sideset " << id_ << ". !  Aborting...\n");
    exit(1);
  }

  numEntity = sets[0].num_entry;
  num_dist_factors = sets[0].num_distribution_factor;
}

template <typename INT>
void
Side_Set<INT>::apply_map(const INT * elmt_map)
{
  SMART_ASSERT(elmt_map != nullptr);
  if (elmts != nullptr)
  {
    delete[] elmts;
    elmts = nullptr;
    delete[] sides;
    sides = nullptr;
    delete[] sideIndex;
    sideIndex = nullptr;
  }
  load_sides(elmt_map);
}

template <typename INT>
void
Side_Set<INT>::load_sides(const INT * elmt_map) const
{
  int err = 0;
  if ((elmts == nullptr || sides == nullptr) && numEntity > 0)
  {
    elmts = new INT[numEntity];
    SMART_ASSERT(elmts != nullptr);
    sides = new INT[numEntity];
    SMART_ASSERT(sides != nullptr);
    sideIndex = new INT[numEntity];
    SMART_ASSERT(sideIndex != nullptr);

    err = ex_get_set(fileId, EX_SIDE_SET, id_, elmts, sides);

    if (err < 0)
    {
      ERROR("Side_Set<INT>::Load_Set(): Failed to read side set " << id_ << "!  Aborting...\n");
      exit(1);
    }

    if (elmt_map != nullptr)
    {
      for (size_t i = 0; i < numEntity; i++)
      {
        elmts[i] = 1 + elmt_map[elmts[i] - 1];
      }
    }

    if (interface.ssmap_flag)
    {
      for (size_t i = 0; i < numEntity; i++)
      {
        sideIndex[i] = i;
        elmts[i] = elmts[i] * 8 + sides[i];
      }

      index_qsort(elmts, sideIndex, numEntity);

      // Recover elmts...
      for (size_t i = 0; i < numEntity; i++)
      {
        elmts[i] = elmts[i] / 8;
      }
    }
    else
    {
      for (size_t i = 0; i < numEntity; i++)
      {
        sideIndex[i] = i;
      }
    }
    SMART_ASSERT(Check_State());
  }
}

template <typename INT>
void
Side_Set<INT>::load_df() const
{
  if (elmts == nullptr)
  {
    load_sides();
  }

  if (dist_factors != nullptr)
  {
    return; // Already loaded.
  }

  dfIndex = new INT[numEntity + 1];
  SMART_ASSERT(dfIndex != nullptr);
  std::vector<int> count(numEntity);

  // Handle the sierra "universal side set" which only has a single df per face...
  if (num_dist_factors == numEntity)
  {
    for (size_t i = 0; i < numEntity; i++)
    {
      count[i] = 1;
    }
  }
  else
  {
    int err = ex_get_side_set_node_count(fileId, id_, count.data());
    if (err < 0)
    {
      ERROR("Side_Set::load_df(): Failed to read side set node count for sideset "
            << id_ << "!  Aborting...\n");
      exit(1);
    }
  }

  // Convert raw counts to index...
  size_t index = 0;
  for (size_t i = 0; i < numEntity; i++)
  {
    dfIndex[i] = index;
    index += count[i];
  }
  dfIndex[numEntity] = index;

  // index value should now equal df count for this sideset...
  if (index != num_dist_factors)
  {
    ERROR("Side_Set::load_df(): Mismatch in distribution factor count for sideset "
          << id_ << ", file says there should be " << num_dist_factors
          << ",\n\t\tbut ex_get_side_set_node_count says there should be " << index
          << "!  Aborting...\n");
    exit(1);
  }
  SMART_ASSERT(index == num_dist_factors);
  dist_factors = new double[index];
  int err = ex_get_set_dist_fact(fileId, EX_SIDE_SET, id_, dist_factors);
  if (err < 0)
  {
    ERROR("Side_Set::load_df(): Failed to read side set distribution factors for sideset "
          << id_ << "!  Aborting...\n");
    exit(1);
  }
}

template <typename INT>
const INT *
Side_Set<INT>::Elements() const
{
  load_sides();
  return elmts;
}

template <typename INT>
const INT *
Side_Set<INT>::Sides() const
{
  load_sides();
  return sides;
}

template <typename INT>
std::pair<INT, INT>
Side_Set<INT>::Side_Id(size_t position) const
{
  load_sides();
  SMART_ASSERT(position < numEntity);
  return std::make_pair(elmts[sideIndex[position]], sides[sideIndex[position]]);
}

template <typename INT>
size_t
Side_Set<INT>::Side_Index(size_t position) const
{
  load_sides();
  SMART_ASSERT(position < numEntity);
  return sideIndex[position];
}

template <typename INT>
const double *
Side_Set<INT>::Distribution_Factors() const
{
  if (dist_factors == nullptr)
  {
    load_df();
  }
  return dist_factors;
}

template <typename INT>
void
Side_Set<INT>::Free_Distribution_Factors() const
{
  if (dist_factors)
  {
    delete[] dist_factors;
    dist_factors = nullptr;
  }
}

template <typename INT>
std::pair<INT, INT>
Side_Set<INT>::Distribution_Factor_Range(size_t side) const
{
  if (dfIndex == nullptr)
  {
    load_df();
  }
  if (dfIndex == nullptr)
  {
    ERROR("Failed to get distribution factors for sideset " << id_ << ". !  Aborting...\n");
    exit(1);
  }
  size_t side_index = sideIndex[side];
  return std::make_pair(dfIndex[side_index], dfIndex[side_index + 1]);
}

template <typename INT>
void
Side_Set<INT>::Display(std::ostream & s) const
{
  SMART_ASSERT(Check_State());

  s << "Side_Set<INT>::Display()  Exodus side set ID = " << id_ << '\n'
    << "                        number of sides = " << numEntity << '\n'
    << "         number of distribution factors = " << num_dist_factors << '\n'
    << "                    number of variables = " << var_count() << '\n';
}

template <typename INT>
int
Side_Set<INT>::Check_State() const
{
  SMART_ASSERT(id_ >= EX_INVALID_ID);
  SMART_ASSERT(!(id_ == EX_INVALID_ID && numEntity > 0));
  SMART_ASSERT(!(id_ == EX_INVALID_ID && num_dist_factors > 0));
  SMART_ASSERT(!(id_ == EX_INVALID_ID && elmts));
  SMART_ASSERT(!(id_ == EX_INVALID_ID && sides));
  SMART_ASSERT(!(id_ == EX_INVALID_ID && dist_factors));

  SMART_ASSERT(!(elmts && !sides));
  SMART_ASSERT(!(!elmts && sides));

  return 1;
}

template class Side_Set<int>;
template class Side_Set<int64_t>;
