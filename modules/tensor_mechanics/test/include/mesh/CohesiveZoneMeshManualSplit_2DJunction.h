/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef COHESIVEZONEMESHMANUALSPLIT_2DJUNCTION_H
#define COHESIVEZONEMESHMANUALSPLIT_2DJUNCTION_H

#include "CohesiveZoneMeshManualSplitBase.h"

// forward declaration
class CohesiveZoneMeshManualSplit_2DJunction;

template <>
InputParameters validParams<CohesiveZoneMeshManualSplit_2DJunction>();

class CohesiveZoneMeshManualSplit_2DJunction : public CohesiveZoneMeshManualSplitBase
{
public:
  CohesiveZoneMeshManualSplit_2DJunction(const InputParameters & parameters);
  CohesiveZoneMeshManualSplit_2DJunction(const CohesiveZoneMeshManualSplit_2DJunction & other_mesh);
  virtual ~CohesiveZoneMeshManualSplit_2DJunction(); // empty dtor required for unique_ptr with
                                                     // forward declarations

  virtual void init() override;

  virtual std::unique_ptr<MooseMesh>  safeClone() const override;

private:
  void updateElements();
  void addInterfaceBoundary();

};

#endif // COHESIVEZONEMESHMANUALSPLIT_2DJUNCTION
