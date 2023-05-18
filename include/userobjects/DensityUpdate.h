#pragma once

#include "ElementUserObject.h"
#include "MooseTypes.h"

class DensityUpdate : public ElementUserObject
{
public:
  static InputParameters validParams();

  DensityUpdate(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void timestepSetup() override;
  virtual void execute() override;
  virtual void finalize() override{};
  virtual void threadJoin(const UserObject &) override{};

protected:
  MooseMesh & _mesh;
  const VariableName _design_density_name;
  const VariableName _density_sensitivity_name;

  MooseVariable & _design_density;
  const MooseVariable & _density_sensitivity;

  const Real _volume_fraction;

private:
  struct ElementData
  {
    Real density;
    Real sensitivity;
    Real volume;
    Real curr_filtered_density;
    ElementData() = default;
    ElementData(Real dens, Real sens, Real vol, Real filt_dens)
      : density(dens), sensitivity(sens), volume(vol), curr_filtered_density(filt_dens)
    {
    }
  };

  void gatherElementData();
  void performOptimCritLoop();
  void computePhysDensity();

  Real computeUpdatedDensity(Real current_density, Real dc, Real lmid);

  Real _total_allowable_volume;
  std::vector<Point> _elem_centroids;
  std::vector<dof_id_type> _elem_id;
  std::map<dof_id_type, ElementData> _elem_data_map;
};
