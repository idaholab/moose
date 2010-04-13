#ifndef FACEDATA_H
#define FACEDATA_H

//libMesh includes

//Forward Declarations
class MooseSystem;

class FaceData
{
public:
  FaceData(MooseSystem & moose_system);

private:
  /**
   * The MooseSystem this Kernel is associated with.
   */
  MooseSystem & _moose_system;
};

  

#endif //FACEDATA_H
