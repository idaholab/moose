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

#ifndef COMPUTEINDICATORTHREAD_H
#define COMPUTEINDICATORTHREAD_H

#include "ThreadedElementLoop.h"

// libMesh includes
#include "libmesh/elem_range.h"

// Forward declarations
class AuxiliarySystem;
class InternalSideIndicators;

class ComputeIndicatorThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  /**
   * @param fe_problem reference to the FEProblemBase we are computing on
   * @param sys reference to the AuxSystem we are computing on
   * @param indicator_whs Warehouse of Indicator objects.
   * @param finalize Whether or not we are just in the "finalize" stage or not.
   */
  ComputeIndicatorThread(FEProblemBase & fe_problem, bool finalize = false);

  // Splitting Constructor
  ComputeIndicatorThread(ComputeIndicatorThread & x, Threads::split split);

  virtual ~ComputeIndicatorThread();

  virtual void subdomainChanged() override;
  virtual void onElement(const Elem * elem) override;
  virtual void onBoundary(const Elem * elem, unsigned int side, BoundaryID bnd_id) override;
  virtual void onInternalSide(const Elem * elem, unsigned int side) override;
  virtual void postElement(const Elem * /*elem*/) override;
  virtual void post() override;

  void join(const ComputeIndicatorThread & /*y*/);

protected:
  FEProblemBase & _fe_problem;
  AuxiliarySystem & _aux_sys;

  /// Indicator Storage
  const MooseObjectWarehouse<Indicator> & _indicator_whs;

  /// InternalSideIndicator Storage
  const MooseObjectWarehouse<InternalSideIndicator> & _internal_side_indicators;

  bool _finalize;
};

#endif // COMPUTEINDICATORTHREAD_H
