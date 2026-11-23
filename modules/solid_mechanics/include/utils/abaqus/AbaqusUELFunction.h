//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

namespace Abaqus
{
/// function type for the external Abaqus UEL plugin function
using UELFunction = void (*)(
    Real RHS[],    // (MLVARX,*)      Residual vector contribution for the current element
    Real AMATRX[], // (NDOFEL,NDOFEL) Jacobian contribution for the current element
    Real SVARS[],  // (NSVARS)        Persistent state variable values for the current element
    Real ENERGY[], // (8)             Energy quantities at the start of the current
                   //                 increment (to be updated by the UEL routine)
    int * NDOFEL,  // Number of degrees of freedom (DOFs) for the current element
    int * NRHS,    // NRHS=1: RHS should contain the residual vector,
                   // NRHS=2: not implemented (modified Riks static procedure)
    int * NSVARS,  // Number of persistent state variables for the element
    Real PROPS[],  // (NPROPS) Static property values (parameters) defined for use with this
                   // element.
    int * NPROPS,  //
    Real COORDS[], // (MCRD,NNODE) Undisplaced coordinates of the element nodes
                   //              COORDS(K1,K2) is the K1th coordinate of the
                   //              K2th node of the element
    int * MCRD,    // Maximum number of coordinates needed at any node point (COORDINATES keyword -
                   // unsupported)
    int * NNODE,   // Number of nodes in the current element
    Real U[],      // (NDOFEL)   Total values of the variables
    Real DU[],     // (MLVARX,*) Incremental values of the variables for the current increment
                   //            for right-hand-side
    Real V[],      // (NDOFEL) Time rate of change of the variables (velocities,
                   //          rates of rotation). Defined for implicit dynamics only (LFLAGS(1)
                   //          11 or 12)
    Real A[],      // (NDOFEL) Accelerations of the variables. Defined for implicit dynamics
                   //          only (LFLAGS(1) 11 or 12).
    int * JTYPE,   // Integer defining the element type. This is the user-defined integer value n in
                   // element type Un
    Real TIME[],   // (2) step time and total time
    Real * DTIME,  // Time increment
    int * KSTEP,   // Step number (as per Abaqus definition) can be set by the user
    int * KINC,    // Increment number (MOOSE time step)
    int * JELEM,   // User-defined element number
    Real PRAMS[],  // (*) parameters associated with the solution procedure
    int * NDLOAD,  // Number of distributed loads applied to the element
    int JDLTYP[],  // (MDLOAD, *) array containing the integers used to define distributed load
                   //             types for the element
    Real ADLMAG[], // (MDLOAD,*)
    Real PREDEF[], // (2,NPREDF,NNODE) predefined field variables, such as temperature in an
                   //                  uncoupled stress/displacement analysis
    int * NPREDF,  // Number of predefined field (auxiliary) variables, including temperature
    int LFLAGS[],  // (*) flags that define the current solution procedure
    int * MLVARX,  // used when several displacement or right-hand-side vectors are used
    Real DDLMAG[], // (MDLOAD,*)
    int * MDLOAD,  // Total number of distributed loads and/or fluxes defined on this element
    Real * PNEWDT, // Recommended ratio of the new to the current time increment
    int JPROPS[],  // (NJPROP) NJPROP integer property values defined for the current element
    int * NJPROP,  // Number of user defined integer properties
    Real * PERIOD  // Current step time period (unused)
);

/**
 * The arguments of one call of an Abaqus UEL plugin. Scalars are held by value and passed by
 * pointer, so values the plugin writes (for example PNEWDT) are read back from this struct after
 * call(). Arrays are pointers into buffers owned by the caller.
 */
struct UELArguments
{
  Real * rhs = nullptr;
  Real * amatrx = nullptr;
  Real * svars = nullptr;
  Real * energy = nullptr;
  int ndofel = 0;
  int nrhs = 1;
  int nsvars = 0;
  Real * props = nullptr;
  int nprops = 0;
  Real * coords = nullptr;
  int mcrd = 0;
  int nnode = 0;
  Real * u = nullptr;
  Real * du = nullptr;
  Real * v = nullptr;
  Real * a = nullptr;
  int jtype = 0;
  Real * time = nullptr;
  Real dtime = 0;
  int kstep = 0;
  int kinc = 0;
  int jelem = 0;
  Real * params = nullptr;
  int ndload = 0;
  int * jdltyp = nullptr;
  Real * adlmag = nullptr;
  Real * predef = nullptr;
  int npredf = 0;
  int * lflags = nullptr;
  int mlvarx = 0;
  Real * ddlmag = nullptr;
  int mdload = 0;
  Real pnewdt = 0;
  int * jprops = nullptr;
  int njprop = 0;
  Real period = 0;

  /// call the plugin with these arguments
  void call(UELFunction uel);
};
}

inline void
Abaqus::UELArguments::call(Abaqus::UELFunction uel)
{
  uel(rhs,
      amatrx,
      svars,
      energy,
      &ndofel,
      &nrhs,
      &nsvars,
      props,
      &nprops,
      coords,
      &mcrd,
      &nnode,
      u,
      du,
      v,
      a,
      &jtype,
      time,
      &dtime,
      &kstep,
      &kinc,
      &jelem,
      params,
      &ndload,
      jdltyp,
      adlmag,
      predef,
      &npredf,
      lflags,
      &mlvarx,
      ddlmag,
      &mdload,
      &pnewdt,
      jprops,
      &njprop,
      &period);
}
