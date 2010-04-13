#ifndef AUXDATA_H
#define AUXDATA_H

//Moose includes

//libMesh includes

//Forward Declarations
class MooseSystem;
class ElementData;

/**
 * One stop shop for all the data an AuxKernel class needs.
 *
 * _One_ of these will get built for each MooseSystem.
 */
class AuxData
{
public:
  AuxData(MooseSystem & moose_system, ElementData & element_data);

private:
  /**
   * The MooseSystem this Kernel is associated with.
   */
  MooseSystem & _moose_system;

  /**
   * Element Data
   */
  ElementData & _element_data;
};

#endif //AUXDATA_H
