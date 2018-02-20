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
#ifndef EXODIFF_MAP_H
#define EXODIFF_MAP_H
#include "exoII_read.h"

enum MAP_TYPE_enum
{
  FILE_ORDER = 0,
  PARTIAL,
  USE_FILE_IDS,
  DISTANCE
};

template <typename INT>
void
Compute_Maps(INT *& node_map, INT *& elmt_map, ExoII_Read<INT> & file1, ExoII_Read<INT> & file2);

template <typename INT>
void Compute_Partial_Maps(INT *& node_map,
                          INT *& elmt_map,
                          ExoII_Read<INT> & file1,
                          ExoII_Read<INT> & file2);

template <typename INT>
void Compute_FileId_Maps(INT *& node_map,
                         INT *& elmt_map,
                         ExoII_Read<INT> & file1,
                         ExoII_Read<INT> & file2);

template <typename INT>
void Dump_Maps(const INT * node_map, const INT * elmt_map, ExoII_Read<INT> & file1);

template <typename INT>
bool Check_Maps(const INT * node_map,
                const INT * elmt_map,
                const ExoII_Read<INT> & file1,
                const ExoII_Read<INT> & file2);

template <typename INT>
bool Compare_Maps(ExoII_Read<INT> & file1,
                  ExoII_Read<INT> & file2,
                  const INT * node_map,
                  const INT * elmt_map,
                  bool partial_flag);

#endif
