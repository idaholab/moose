#ifndef EBSDACCESSFUNCTORS_H
#define  EBSDACCESSFUNCTORS_H

class EBSDAccessFunctors
{
public:
  /// Per element EBSD data point
  struct EBSDPointData {
    Real phi1, phi, phi2, phase, symmetry;
    unsigned int grain, op;
    Point p;
  };

  /// Averaged EBSD data
  struct EBSDAvgData {
    Real phi1, phi, phi2, phase, symmetry;
    unsigned int n;
    Point p;
  };

  /// Access functor base class for EBSDPointData
  struct EBSDPointDataFunctor {
    virtual Real operator () (const EBSDPointData &) = 0;
  };

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
};

#endif //EBSDACCESSFUNCTORS_H
