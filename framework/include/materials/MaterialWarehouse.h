//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseObjectWarehouse.h"

// Forward declarations
class MaterialBase;

/**
 * MaterialBase objects are special in that they have additional objects created automatically (see
 * FEProblemBase::addMaterial).
 *
 * This class specializes the base class to acount for the additional Neightbor and face objects
 * that may
 * exist.
 */
class MaterialWarehouse : public MooseObjectWarehouse<MaterialBase>
{
public:
  const MooseObjectWarehouse<MaterialBase> & operator[](Moose::MaterialDataType data_type) const;

  ///@{
  /**
   * Convenience methods for calling object setup methods that handle the extra neighbor and face
   * objects.
   */
  virtual void initialSetup(THREAD_ID tid = 0) const;
  virtual void timestepSetup(THREAD_ID tid = 0) const;
  virtual void subdomainSetup(THREAD_ID tid = 0) const;
  virtual void subdomainSetup(SubdomainID id, THREAD_ID tid = 0) const;
  virtual void neighborSubdomainSetup(THREAD_ID tid = 0) const;
  virtual void neighborSubdomainSetup(SubdomainID id, THREAD_ID tid = 0) const;
  virtual void jacobianSetup(THREAD_ID tid = 0) const;
  virtual void residualSetup(THREAD_ID tid = 0) const;
  virtual void updateActive(THREAD_ID tid = 0);
  void sort(THREAD_ID tid = 0);
  ///@}

  /**
   * A special method unique to this class for adding Block, Neighbor, and Face material objects.
   */
  void addObjects(std::shared_ptr<MaterialBase> block,
                  std::shared_ptr<MaterialBase> neighbor,
                  std::shared_ptr<MaterialBase> face,
                  THREAD_ID tid = 0);

  /**
   * A special method unique to this class for adding Interface material objects.
   */
  void addInterfaceObject(std::shared_ptr<MaterialBase> interface, THREAD_ID tid = 0);

protected:
  /// Storage for neighbor material objects (Block are stored in the base class)
  MooseObjectWarehouse<MaterialBase> _neighbor_materials;

  /// Storage for face material objects (Block are stored in the base class)
  MooseObjectWarehouse<MaterialBase> _face_materials;

  /// Storage for interface material objects
  MooseObjectWarehouse<MaterialBase> _interface_materials;
};
