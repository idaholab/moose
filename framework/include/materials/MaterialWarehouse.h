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

#ifndef MATERIALWAREHOUSE_H
#define MATERIALWAREHOUSE_H

// MOOSE includes
#include "MooseObjectWarehouse.h"

// Forward declarations
class Material;

/**
 * Material objects are special in that they have additional objects created automatically (see FEProblem::addMaterial).
 *
 * This class specializes the base class to acount for the additional Neightbor and face objects that may
 * exist.
 */
class MaterialWarehouse : public MooseObjectWarehouse<Material>
{
public:
  const MooseObjectWarehouse<Material> & operator[](Moose::MaterialDataType data_type) const;

  ///@{
  /**
   * Convenience methods for calling object setup methods that handle the extra neighbor and face objects.
   */
  virtual void initialSetup(THREAD_ID tid = 0) const;
  virtual void timestepSetup(THREAD_ID tid = 0) const;
  virtual void subdomainSetup(THREAD_ID tid = 0) const;
  virtual void subdomainSetup(SubdomainID id, THREAD_ID tid = 0) const;
  virtual void jacobianSetup(THREAD_ID tid = 0) const;
  virtual void residualSetup(THREAD_ID tid = 0) const;
  virtual void updateActive(THREAD_ID tid = 0);
  void sort(THREAD_ID tid = 0);
  ///@}

  /**
   * A special method unique to this class for adding Block, Neighbor, and Face material objects.
   */
  void addObjects(MooseSharedPointer<Material> block, MooseSharedPointer<Material> neighbor, MooseSharedPointer<Material> face, THREAD_ID tid = 0);

protected:
  /// Stroage for neighbor material objects (Block are stored in the base class)
  MooseObjectWarehouse<Material> _neighbor_materials;

  /// Stroage for face material objects (Block are stored in the base class)
  MooseObjectWarehouse<Material> _face_materials;
};

#endif // MATERIALWAREHOUSE_H
