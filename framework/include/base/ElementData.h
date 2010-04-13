#ifndef ELEMENTDATA_H
#define ELEMENTDATA_H

//MOOSE includes

//libMesh includes

//Forward Declarations
class MooseSystem;

/**
 * One stop shop for all the data a Kernel class needs.
 *
 * _One_ of these will get built for each MooseSystem.
 */
class ElementData
{
public:
  ElementData(MooseSystem & moose_system);

private:
  /**
   * The MooseSystem this Kernel is associated with.
   */
  MooseSystem & _moose_system;
};

#endif //ELEMENTDATA_H
