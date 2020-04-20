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
#include "iqsort.h"
#include <cstdint>

namespace {
template <typename INT>
void swap_(INT v[], size_t i, size_t j);

template <typename T, typename INT>
INT median3(const T v[], INT iv[], size_t left, size_t right);

template <typename T, typename INT>
void iqsort(const T v[], INT iv[], size_t left, size_t right);

template <typename T, typename INT>
void iisort(const T v[], INT iv[], size_t N);

template <typename T, typename INT>
void check(const T v[], INT iv[], size_t N);
} // namespace

template <typename T, typename INT>
void
index_qsort(const T v[], INT iv[], size_t N)
{
  if (N <= 1)
  {
    return;
  }
  iqsort(v, iv, 0, N - 1);
  iisort(v, iv, N);
  check(v, iv, N);
}

// The following are not part of the public interface...

namespace {
/* The following 'indexed qsort' routine is modified from Sedgewicks
 * algorithm It selects the pivot based on the median of the left,
 * right, and center values to try to avoid degenerate cases ocurring
 * when a single value is chosen.  It performs a quicksort on
 * intervals down to the QSORT_CUTOFF size and then performs a final
 * insertion sort on the almost sorted final array.  Based on data in
 * Sedgewick, the QSORT_CUTOFF value should be between 5 and 20.
 *
 * See Sedgewick for further details Define DEBUG_QSORT at the top of
 * this file and recompile to compile in code that verifies that the
 * array is sorted.
 */

#define QSORT_CUTOFF 12

/* swap - interchange v[i] and v[j] */
template <typename INT>
void
swap_(INT v[], size_t i, size_t j)
{
  INT temp;

  temp = v[i];
  v[i] = v[j];
  v[j] = temp;
}

template <typename T, typename INT>
INT
median3(const T v[], INT iv[], size_t left, size_t right)
{
  size_t center = (left + right) / 2;

  if (v[iv[left]] > v[iv[center]])
  {
    swap_(iv, left, center);
  }
  if (v[iv[left]] > v[iv[right]])
  {
    swap_(iv, left, right);
  }
  if (v[iv[center]] > v[iv[right]])
  {
    swap_(iv, center, right);
  }

  swap_(iv, center, right - 1);
  return iv[right - 1];
}

template <typename T, typename INT>
void
iqsort(const T v[], INT iv[], size_t left, size_t right)
{
  if (left + QSORT_CUTOFF <= right)
  {
    size_t pivot = median3(v, iv, left, right);
    size_t i = left;
    size_t j = right - 1;

    for (;;)
    {
      while (v[iv[++i]] < v[pivot])
      {
        ;
      }
      while (v[iv[--j]] > v[pivot])
      {
        ;
      }
      if (i < j)
      {
        swap_(iv, i, j);
      }
      else
      {
        break;
      }
    }

    swap_(iv, i, right - 1);
    iqsort(v, iv, left, i - 1);
    iqsort(v, iv, i + 1, right);
  }
  }

  template <typename T, typename INT>
  void
  iisort(const T v[], INT iv[], size_t N)
  {
    size_t j;
    size_t ndx = 0;

    T small = v[iv[0]];
    for (size_t i = 1; i < N; i++)
    {
      if (v[iv[i]] < small)
      {
        small = v[iv[i]];
        ndx = i;
      }
    }
    /* Put smallest value in slot 0 */
    swap_(iv, 0, ndx);

    for (size_t i = 1; i < N; i++)
    {
      INT tmp = iv[i];
      for (j = i; v[tmp] < v[iv[j - 1]]; j--)
      {
        iv[j] = iv[j - 1];
      }
      iv[j] = tmp;
    }
  }

  template <typename T, typename INT>
  void
  check(const T v[], INT iv[], size_t N)
  {
#if defined(DEBUG_QSORT)
    fprintf(stderr, "Checking sort of %d values\n", N + 1);
    size_t i;
    for (i = 1; i < N; i++)
    {
      SMART_ASSERT(v[iv[i - 1]] <= v[iv[i]]);
    }
#endif
  }
  } // namespace

  template void index_qsort(const int v[], int iv[], size_t N);
  template void index_qsort(const double v[], int iv[], size_t N);

  template void index_qsort(const int64_t v[], int64_t iv[], size_t N);
  template void index_qsort(const double v[], int64_t iv[], size_t N);
