/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EBSDACCESSFUNCTORS_H
#define EBSDACCESSFUNCTORS_H

#include "MooseObject.h"
#include "MooseEnum.h"
#include "EulerAngles.h"

/**
 * Mix-in class that adds so called access functors to select a field from
 * an EBSDPointData or EBSDPointData (todo) structure. The field name is specified
 * by name as a std::string.
 */
class EBSDAccessFunctors
{
public:
  /// Per element EBSD data point
  struct EBSDPointData {
    Real phi1, Phi, phi2;
    unsigned int symmetry, grain, phase, op, global;
    Point p;
    std::vector<Real> custom;
  };

  /// Averaged EBSD data
  struct EBSDAvgData {
    EulerAngles * angles;
    unsigned int phase, local, symmetry, grain, n;
    Point p;
    std::vector<Real> custom;
  };

  static MooseEnum getPointDataFieldType();
  static MooseEnum getAvgDataFieldType();

  /// Access functor base class for EBSDPointData
  struct EBSDPointDataFunctor {
    virtual Real operator () (const EBSDPointData &) = 0;
    virtual ~EBSDPointDataFunctor() {};
  };
  /// Access functor base class for EBSDAvgData
  struct EBSDAvgDataFunctor {
    virtual Real operator () (const EBSDAvgData &) = 0;
    virtual ~EBSDAvgDataFunctor() {};
  };

  // List of specialized access functors (one for each field in EBSDPointData)
  struct EBSDPointDataPhi1 : EBSDPointDataFunctor {
    virtual Real operator () (const EBSDPointData & d) { return d.phi1; };
  };
  struct EBSDPointDataPhi : EBSDPointDataFunctor {
    virtual Real operator () (const EBSDPointData & d) { return d.Phi; };
  };
  struct EBSDPointDataPhi2 : EBSDPointDataFunctor {
    virtual Real operator () (const EBSDPointData & d) { return d.phi2; };
  };
  struct EBSDPointDataPhase : EBSDPointDataFunctor {
    virtual Real operator () (const EBSDPointData & d) { return d.phase; };
  };
  struct EBSDPointDataSymmetry : EBSDPointDataFunctor {
    virtual Real operator () (const EBSDPointData & d) { return d.symmetry; };
  };
  struct EBSDPointDataGrain : EBSDPointDataFunctor {
    virtual Real operator () (const EBSDPointData & d) { return d.grain; };
  };
  struct EBSDPointDataOp : EBSDPointDataFunctor {
    virtual Real operator () (const EBSDPointData & d) { return d.op; };
  };
  struct EBSDPointDataCustom : EBSDPointDataFunctor {
    EBSDPointDataCustom(unsigned int index) : _index(index) {}
    virtual Real operator () (const EBSDPointData & d) { mooseAssert(_index < d.custom.size(), "Requesting out of bounds index in EBSDPointDataCustom."); return d.custom[_index]; };
    const unsigned int _index;
  };

  // List of specialized access functors (one for each field in EBSDAvgData)
  struct EBSDAvgDataPhi1 : EBSDAvgDataFunctor {
    virtual Real operator () (const EBSDAvgData & a) { return a.angles->phi1; };
  };
  struct EBSDAvgDataPhi : EBSDAvgDataFunctor {
    virtual Real operator () (const EBSDAvgData & a) { return a.angles->Phi; };
  };
  struct EBSDAvgDataPhi2 : EBSDAvgDataFunctor {
    virtual Real operator () (const EBSDAvgData & a) { return a.angles->phi2; };
  };
  struct EBSDAvgDataPhase : EBSDAvgDataFunctor {
    virtual Real operator () (const EBSDAvgData & a) { return a.phase; };
  };
  struct EBSDAvgDataLocalID : EBSDAvgDataFunctor {
    virtual Real operator () (const EBSDAvgData & a) { return a.local; };
  };
  struct EBSDAvgDataGrain : EBSDAvgDataFunctor {
    virtual Real operator () (const EBSDAvgData & a) { return a.grain; };
  };
  struct EBSDAvgDataSymmetry : EBSDAvgDataFunctor {
    virtual Real operator () (const EBSDAvgData & a) { return a.symmetry; };
  };
  struct EBSDAvgDataCustom : EBSDAvgDataFunctor {
    EBSDAvgDataCustom(unsigned int index) : _index(index) {}
    virtual Real operator () (const EBSDAvgData & a) { mooseAssert(_index < a.custom.size(), "Requesting out of bounds index in EBSDPointDataCustom."); return a.custom[_index]; };
    const unsigned int _index;
  };
};

#endif //EBSDACCESSFUNCTORS_H
