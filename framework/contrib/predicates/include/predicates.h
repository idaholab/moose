/*****************************************************************************/
/*                                                                           */
/*  Declarations for the 2D subset of                                        */
/*                                                                           */
/*    Routines for Arbitrary Precision Floating-point Arithmetic             */
/*    and Fast Robust Geometric Predicates                                   */
/*                                                                           */
/*  Placed in the public domain by Jonathan Richard Shewchuk.  See           */
/*  http://www.cs.cmu.edu/~quake/robust.html and the vendoring notes at the  */
/*  top of predicates.c.                                                     */
/*                                                                           */
/*  This header is written for both C and C++ translation units, because     */
/*  predicates.c is compiled as C.                                           */
/*                                                                           */
/*****************************************************************************/

#ifndef PREDICATES_H
#define PREDICATES_H

#ifdef __cplusplus
extern "C"
{
#endif

  /* These three keep external linkage because C++ translation units call them,
     so they carry a moose_ prefix over their upstream names: libMesh's bundled
     Triangle exports an exactinit() and a six-argument incircle() of its own,
     and binding to those would leave the error bounds here at zero and call
     incircle() with the wrong number of arguments.  See vendoring note 4 at the
     top of predicates.c. */

  /* Initialize the shared roundoff constants and error bounds.  This must run
     exactly once before any predicate below is called.  C++ callers should use
     initPredicates() rather than calling this directly. */
  void moose_exactinit(void);

  /* Exact sign of (pb[0] - pa[0]) * (pc[1] - pa[1]) - (pb[1] - pa[1]) *
     (pc[0] - pa[0]): positive if pa, pb, pc occur in counterclockwise order,
     negative if clockwise, and zero if they are collinear.  The magnitude is
     an approximation of twice the signed triangle area. */
  double moose_orient2d(const double * pa, const double * pb, const double * pc);

  /* Exact sign of the in-circle determinant: positive if pd lies inside the
     circle through pa, pb and pc, negative if it lies outside, and zero if the
     four points are cocircular.  pa, pb and pc must be in counterclockwise
     order, or the sign is reversed. */
  double
  moose_incircle(const double * pa, const double * pb, const double * pc, const double * pd);

#ifdef __cplusplus
}

#include <mutex>

/**
 * Runs moose_exactinit() the first time it is called, and never again. Every caller of
 * moose_orient2d() or moose_incircle() must call this first; the predicates read shared constants
 * that moose_exactinit() writes.
 */
inline void
initPredicates()
{
  static std::once_flag flag;
  std::call_once(flag, []() { moose_exactinit(); });
}

#endif

#endif
