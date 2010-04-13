#ifndef MOOSESYSTEM_H
#define MOOSESYSTEM_H

#include <vector>

//MOOSE includes
#include "Moose.h"  // for THREAD_ID

//libmesh includes
#include "transient_system.h"

//Forward Declarations
class EquationSystems;
class ElementData;
class MeshBase;
class FaceData;
class AuxData;
class Material;
template<class T> class NumericVector;

/**
 * This class represents one full coupled system of nonlinear equations including any
 * explicit (Aux) equations.
 *
 * You can have multiple MOOSE systems... and (one day) couple them together).
 */
class MooseSystem
{
public:
  MooseSystem();
  ~MooseSystem();

  /**
   * Data Accessors for the various FE datastructures indexed by thread
   */
  ElementData * getElementData(THREAD_ID tid);
  FaceData * getFaceData(THREAD_ID tid);
  AuxData * getAuxData(THREAD_ID tid);

  /**
   * Initialize the Mesh for this MooseSystem and return a reference
   */
  MeshBase * initMesh(unsigned int dim);

  /**
   * Returns a writable reference to the mesh held wihin this MooseSystem
   */
  MeshBase * getMesh();

  inline unsigned int getDim() { return _dim; }
  
  /**
   * Initialize the EquationSystems object and add both the nonlinear and auxiliary systems
   * to that object for this MooseSystem
   */
  EquationSystems * initEquationSystems();

  /**
   * Returns a writable reference to the EquationSystems object helf within this MooseSystem
   */
  EquationSystems * getEquationSystems();

  /**
   * Returns a reference to the main nonlinear system in this instance of MooseSystem
   */
  TransientNonlinearImplicitSystem * getNonlinearSystem();
  
  /**
   * Returns a reference to the auxillary system in this instance of MooseSystem
   */
  TransientExplicitSystem * getAuxSystem();
  
  /**
   * Initialize all of the FE datastructures
   */
  void initDataStructures();

  /**
   * Check to see if MooseSystem is in a workable state before accessing data
   */
  void checkValid();

  /**
   * Get the Exodus Reader for this system.
   */
  ExodusII_IO * getExodusReader();
  
private:
  std::vector<ElementData *> _element_data;
  std::vector<FaceData *> _face_data;
  std::vector<AuxData *> _aux_data;

  EquationSystems * _es;
  TransientNonlinearImplicitSystem * _system;
  TransientExplicitSystem * _aux_system;
  MeshBase * _mesh;
  unsigned int _dim;

  /**
   * The ExodusIO Reader to support reading of solutions at element qps
   */
  ExodusII_IO * _exreader;

  bool _is_valid;
};

#endif //MOOSESYSTEM_H
