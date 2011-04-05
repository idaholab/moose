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
#include "iqsort.h"
#include "smart_assert.h"

namespace {
  void swap_(int v[], int i, int j);

  template <typename T>
  int median3(const T v[], int iv[], int left, int right);

  template <typename T>
  void iqsort(const T v[], int iv[], int left, int right);

  template <typename T>
  void iisort(const T v[], int iv[], int N);

  template <typename T>
  void check(const T v[], int iv[], int N);
}


// Too avoid issues of instantiation, the visible interface
// defines two qsort instances -- double and int.  (Could define more)
// These then cause the correct template functions in this file to be
// instantiated so there should be no linker problems.  Only issue is
// that a new sort type needs to be added below and to iqsort.h instead
// of it appearing magically if index_qsort were a template...

void index_qsort(const double v[], int iv[], int N)
{
  iqsort(v, iv, 0, N-1);
  iisort(v, iv, N);
  check(v, iv, N);
}

void index_qsort(const int v[], int iv[], int N)
{
  iqsort(v, iv, 0, N-1);
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
void swap_(int v[], int i, int j)
{
  int temp;

  temp = v[i];
  v[i] = v[j];
  v[j] = temp;
}

  template <typename T>
  int median3(const T v[], int iv[], int left, int right)
{
  int center;
  center = (left + right) / 2;

  if (v[iv[left]] > v[iv[center]])
    swap_(iv, left, center);
  if (v[iv[left]] > v[iv[right]])
    swap_(iv, left, right);
  if (v[iv[center]] > v[iv[right]])
    swap_(iv, center, right);

  swap_(iv, center, right-1);
  return iv[right-1];
}

  template <typename T>
  void iqsort(const T v[], int iv[], int left, int right)
{
  int pivot;
  int i, j;

  if (left + QSORT_CUTOFF <= right) {
    pivot = median3(v, iv, left, right);
    i = left;
    j = right - 1;

    for ( ; ; ) {
      while (v[iv[++i]] < v[pivot]);
      while (v[iv[--j]] > v[pivot]);
      if (i < j) {
        swap_(iv, i, j);
      } else {
        break;
      }
    }

    swap_(iv, i, right-1);
    iqsort(v, iv, left, i-1);
    iqsort(v, iv, i+1, right);
  }
}

  template <typename T>
  void iisort(const T v[], int iv[], int N)
{
  int i,j;
  int ndx = 0;
  T small;
  int tmp;

  small = v[iv[0]];
  for (i = 1; i < N; i++) {
    if (v[iv[i]] < small) {
      small = v[iv[i]];
      ndx = i;
    }
  }
  /* Put smallest value in slot 0 */
  swap_(iv, 0, ndx);

  for (i=1; i <N; i++) {
    tmp = iv[i];
    for (j=i; v[tmp] < v[iv[j-1]]; j--) {
      iv[j] = iv[j-1];
    }
    iv[j] = tmp;
  }
}

  template <typename T>
  void check(const T v[], int iv[], int N)
  {
#if defined(DEBUG_QSORT)
  fprintf(stderr, "Checking sort of %d values\n", N+1);
  int i;
  for (i=1; i < N; i++) {
    SMART_ASSERT(v[iv[i-1]] <= v[iv[i]]);
  }
#endif
  }
}
