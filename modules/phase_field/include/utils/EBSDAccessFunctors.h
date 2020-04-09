//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "MooseEnum.h"
#include "EulerAngles.h"

#include "libmesh/point.h"

/**
 * Mix-in class that adds so called access functors to select a field from
 * an EBSDPointData or EBSDPointData (todo) structure. The field name is specified
 * by name as a std::string.
 */
class EBSDAccessFunctors
{
public:
  /// Per element EBSD data point
  struct EBSDPointData
  {
    ///@{ Euler angles
    Real _phi1;
    Real _Phi;
    Real _phi2;
    ///@}

    /// Element centroid position
    Point _p;

    ///@{ EBSD feature id, (gklobal) grain number, symmetry, and phase data
    unsigned int _feature_id;
    unsigned int _phase;
    unsigned int _symmetry;
    ///@}

    /// Custom data columns
    std::vector<Real> _custom;
  };

  /// Averaged EBSD data
  struct EBSDAvgData
  {
    /// Averaged Euler angles
    EulerAngles * _angles;

    ///@{ EBSD grain, symmetry, and phase data
    unsigned int _feature_id;
    unsigned int _phase;
    unsigned int _symmetry;
    ///@}

    /// Index in the per-phase list of global IDs
    unsigned int _local_id;

    /// Number of EBSD data points in the global grain
    unsigned int _n;

    /// Center of mass for the global grain
    Point _p;

    /// Grain averaged custom data columns
    std::vector<Real> _custom;
  };

  static MooseEnum getPointDataFieldType();
  static MooseEnum getAvgDataFieldType();

  /// Access functor base class for EBSDPointData
  struct EBSDPointDataFunctor
  {
    virtual Real operator()(const EBSDPointData &) = 0;
    virtual ~EBSDPointDataFunctor(){};
  };
  /// Access functor base class for EBSDAvgData
  struct EBSDAvgDataFunctor
  {
    virtual Real operator()(const EBSDAvgData &) = 0;
    virtual ~EBSDAvgDataFunctor(){};
  };

  // List of specialized access functors (one for each field in EBSDPointData)
  struct EBSDPointDataPhi1 : EBSDPointDataFunctor
  {
    virtual Real operator()(const EBSDPointData & d) { return d._phi1; };
  };
  struct EBSDPointDataPhi : EBSDPointDataFunctor
  {
    virtual Real operator()(const EBSDPointData & d) { return d._Phi; };
  };
  struct EBSDPointDataPhi2 : EBSDPointDataFunctor
  {
    virtual Real operator()(const EBSDPointData & d) { return d._phi2; };
  };
  struct EBSDPointDataFeatureID : EBSDPointDataFunctor
  {
    virtual Real operator()(const EBSDPointData & d) { return d._feature_id; };
  };
  struct EBSDPointDataPhase : EBSDPointDataFunctor
  {
    virtual Real operator()(const EBSDPointData & d) { return d._phase; };
  };
  struct EBSDPointDataSymmetry : EBSDPointDataFunctor
  {
    virtual Real operator()(const EBSDPointData & d) { return d._symmetry; };
  };
  struct EBSDPointDataCustom : EBSDPointDataFunctor
  {
    EBSDPointDataCustom(unsigned int index) : _index(index) {}
    virtual Real operator()(const EBSDPointData & d)
    {
      mooseAssert(_index < d._custom.size(),
                  "Requesting out of bounds index in EBSDPointDataCustom.");
      return d._custom[_index];
    };
    const unsigned int _index;
  };

  // List of specialized access functors (one for each field in EBSDAvgData)
  struct EBSDAvgDataPhi1 : EBSDAvgDataFunctor
  {
    virtual Real operator()(const EBSDAvgData & a) { return a._angles->phi1; };
  };
  struct EBSDAvgDataPhi : EBSDAvgDataFunctor
  {
    virtual Real operator()(const EBSDAvgData & a) { return a._angles->Phi; };
  };
  struct EBSDAvgDataPhi2 : EBSDAvgDataFunctor
  {
    virtual Real operator()(const EBSDAvgData & a) { return a._angles->phi2; };
  };
  struct EBSDAvgDataPhase : EBSDAvgDataFunctor
  {
    virtual Real operator()(const EBSDAvgData & a) { return a._phase; };
  };
  struct EBSDAvgDataSymmetry : EBSDAvgDataFunctor
  {
    virtual Real operator()(const EBSDAvgData & a) { return a._symmetry; };
  };
  struct EBSDAvgDataLocalID : EBSDAvgDataFunctor
  {
    virtual Real operator()(const EBSDAvgData & a) { return a._local_id; };
  };
  struct EBSDAvgDataFeatureID : EBSDAvgDataFunctor
  {
    virtual Real operator()(const EBSDAvgData & a) { return a._feature_id; };
  };
  struct EBSDAvgDataCustom : EBSDAvgDataFunctor
  {
    EBSDAvgDataCustom(unsigned int index) : _index(index) {}
    virtual Real operator()(const EBSDAvgData & a)
    {
      mooseAssert(_index < a._custom.size(),
                  "Requesting out of bounds index in EBSDPointDataCustom.");
      return a._custom[_index];
    };
    const unsigned int _index;
  };
};
