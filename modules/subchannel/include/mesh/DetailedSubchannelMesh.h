#ifndef DETAILEDSUBCHANNELMESH_H
#define DETAILEDSUBCHANNELMESH_H

#include "SubchannelMesh.h"

class DetailedSubchannelMesh;

template <>
InputParameters validParams<DetailedSubchannelMesh>();

class DetailedSubchannelMesh : public SubchannelMesh
{
public:
  DetailedSubchannelMesh(const InputParameters & parameters);

  virtual void buildMesh() override;
};
#endif // DETAILEDSUBCHANNELMESH_H
