//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMSIDENEIGHBORLAYERSVARTESTER_H
#define ELEMSIDENEIGHBORLAYERSVARTESTER_H

#include "ElementUserObject.h"

/**
 * Tests whether a coupled variable is correctly ghosted.
 * This code would benefit from a bit of cleaning.
 **/
class ElemSideNeighborLayersVarTester;

template <>
InputParameters validParams<ElemSideNeighborLayersVarTester>();

class ElemSideNeighborLayersVarTester : public ElementUserObject
{
public:
  ElemSideNeighborLayersVarTester(const InputParameters & parameters);

  /// Cache the nodal values of u that the processor can see
  virtual void timestepSetup() override;

  virtual void execute() override{};
  virtual void initialize() override{};

  /// add _kij contributions
  virtual void threadJoin(const UserObject & /*uo*/) override{};

  virtual void finalize() override;

  /**
   * retrieve the nodal value of u for the global node id given.  If this processor cannot see the
   * nodal values, return -1
   */
  Real getNodalValue(dof_id_type node_id) const;

protected:
  /// the processor rank
  const unsigned int _rank;

  /// the nodal values of u
  MooseVariable * _u_nodal;

  /// variable number for u
  const unsigned int _u_var_num;

  /// the cached data that this processor can see
  std::map<dof_id_type, Real> _nodal_data;
};

#endif // ELEMSIDENEIGHBORLAYERSVARTESTER_H
