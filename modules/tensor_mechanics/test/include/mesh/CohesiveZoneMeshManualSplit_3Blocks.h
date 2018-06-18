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

#ifndef COHESIVEZONEMESHMANUALSPLIT_3BLOCKS_H
#define COHESIVEZONEMESHMANUALSPLIT_3BLOCKS_H

#include "CohesiveZoneMeshManualSplitBase.h"

// forward declaration
class CohesiveZoneMeshManualSplit_3Blocks;

template <>
InputParameters validParams<CohesiveZoneMeshManualSplit_3Blocks>();

class CohesiveZoneMeshManualSplit_3Blocks : public CohesiveZoneMeshManualSplitBase
{
public:
  CohesiveZoneMeshManualSplit_3Blocks(const InputParameters & parameters);
  CohesiveZoneMeshManualSplit_3Blocks(
      const CohesiveZoneMeshManualSplit_3Blocks & other_mesh);
  virtual ~CohesiveZoneMeshManualSplit_3Blocks(); // empty dtor required for unique_ptr with
                                                        // forward declarations

  virtual void init() override;

  virtual std::unique_ptr<MooseMesh>  safeClone() const override;

private:
  void updateElements();
  void addInterfaceBoundary();

};

#endif // COHESIVEZONEMESHMANUALSPLIT_3BLOCKS_H
