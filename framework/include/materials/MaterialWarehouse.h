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

/**
 * Material objects are special in that they have additional objects created automatically (see FEProblem::addMaterial).
 *
 * This class specializes the base class to acount for the additional Neightbor and face objects that may
 * exist.
 */
template<typename T>
class MaterialWarehouse : public MooseObjectWarehouse<T>
{
public:
  const MooseObjectWarehouse<T> & operator[](Moose::MaterialDataType data_type) const;

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
  void addObjects(MooseSharedPointer<T> block, MooseSharedPointer<T> neighbor, MooseSharedPointer<T> face, THREAD_ID tid = 0);

protected:
  /// Stroage for neighbor material objects (Block are stored in the base class)
  MooseObjectWarehouse<T> _neighbor_materials;

  /// Stroage for face material objects (Block are stored in the base class)
  MooseObjectWarehouse<T> _face_materials;
};


template<typename T>
void
MaterialWarehouse<T>::addObjects(MooseSharedPointer<T> block, MooseSharedPointer<T> neighbor, MooseSharedPointer<T> face, THREAD_ID tid /*=0*/)
{
  MooseObjectWarehouse<T>::addObject(block, tid);
  _neighbor_materials.addObject(neighbor, tid);
  _face_materials.addObject(face, tid);
}


template<typename T>
const MooseObjectWarehouse<T> &
MaterialWarehouse<T>::operator[](Moose::MaterialDataType data_type) const
{
  switch (data_type)
  {
  case Moose::NEIGHBOR_MATERIAL_DATA:
    return _neighbor_materials;
    break;
  case Moose::FACE_MATERIAL_DATA:
    return _face_materials;
    break;
  default:
    return *this;
  }
}


template<typename T>
void
MaterialWarehouse<T>::initialSetup(THREAD_ID tid /*=0*/) const
{
  MooseObjectWarehouse<T>::initialSetup(tid);
  _neighbor_materials.initialSetup(tid);
  _face_materials.initialSetup(tid);
}


template<typename T>
void
MaterialWarehouse<T>::timestepSetup(THREAD_ID tid /*=0*/) const
{
  MooseObjectWarehouse<T>::timestepSetup(tid);
  _neighbor_materials.timestepSetup(tid);
  _face_materials.timestepSetup(tid);
}


template<typename T>
void
MaterialWarehouse<T>::subdomainSetup(THREAD_ID tid /*=0*/) const
{
  MooseObjectWarehouse<T>::subdomainSetup(tid);
  _neighbor_materials.subdomainSetup(tid);
  _face_materials.subdomainSetup(tid);
}


template<typename T>
void
MaterialWarehouse<T>::subdomainSetup(SubdomainID id, THREAD_ID tid /*=0*/) const
{
  MooseObjectWarehouse<T>::subdomainSetup(id, tid);
  _neighbor_materials.subdomainSetup(id, tid);
  _face_materials.subdomainSetup(id, tid);
}


template<typename T>
void
MaterialWarehouse<T>::residualSetup(THREAD_ID tid /*=0*/) const
{
  MooseObjectWarehouse<T>::residualSetup(tid);
  _neighbor_materials.residualSetup(tid);
  _face_materials.residualSetup(tid);
}


template<typename T>
void
MaterialWarehouse<T>::jacobianSetup(THREAD_ID tid /*=0*/) const
{
  MooseObjectWarehouse<T>::jacobianSetup(tid);
  _neighbor_materials.jacobianSetup(tid);
  _face_materials.jacobianSetup(tid);
}


template<typename T>
void
MaterialWarehouse<T>::updateActive(THREAD_ID tid /*=0*/)
{
  MooseObjectWarehouse<T>::updateActive(tid);
  _neighbor_materials.updateActive(tid);
  _face_materials.updateActive(tid);
}


template<typename T>
void
MaterialWarehouse<T>::sort(THREAD_ID tid /*=0*/)
{
  MooseObjectWarehouse<T>::sort(tid);
  _neighbor_materials.sort(tid);
  _face_materials.sort(tid);
}

#endif // MATERIALWAREHOUSE_H
