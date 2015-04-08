#ifndef EBSDACCESSFUNCTORS_H
#define EBSDACCESSFUNCTORS_H

#include "MooseObject.h"
#include "MooseEnum.h"

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
    Real phi1, phi, phi2, symmetry;
    unsigned int grain, phase, op;
    Point p;
  };

  /// Averaged EBSD data
  struct EBSDAvgData {
    Real phi1, phi, phi2;
    unsigned int phase, symmetry, grain, n;
    Point p;
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

  /// Factory function to return a point functor specified by name
  static EBSDPointDataFunctor * getPointDataAccessFunctor(const MooseEnum & field_name);
  /// Factory function to return a average functor specified by name
  static EBSDAvgDataFunctor * getAvgDataAccessFunctor(const MooseEnum & field_name);

  // List of specialized access functors (one for each field in EBSDPointData)
  struct EBSDPointDataPhi1 : EBSDPointDataFunctor {
    virtual Real operator () (const EBSDPointData & d) { return d.phi1; };
  };
  struct EBSDPointDataPhi : EBSDPointDataFunctor {
    virtual Real operator () (const EBSDPointData & d) { return d.phi; };
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

  // List of specialized access functors (one for each field in EBSDAvgData)
  struct EBSDAvgDataPhi1 : EBSDAvgDataFunctor {
    virtual Real operator () (const EBSDAvgData & a) { return a.phi1; };
  };
  struct EBSDAvgDataPhi : EBSDAvgDataFunctor {
    virtual Real operator () (const EBSDAvgData & a) { return a.phi; };
  };
  struct EBSDAvgDataPhi2 : EBSDAvgDataFunctor {
    virtual Real operator () (const EBSDAvgData & a) { return a.phi2; };
  };
  struct EBSDAvgDataPhase : EBSDAvgDataFunctor {
    virtual Real operator () (const EBSDAvgData & a) { return a.phase; };
  };
  struct EBSDAvgDataSymmetry : EBSDAvgDataFunctor {
    virtual Real operator () (const EBSDAvgData & a) { return a.symmetry; };
  };
};

#endif //EBSDACCESSFUNCTORS_H
