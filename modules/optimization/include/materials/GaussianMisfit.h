#pragma once

#include "Material.h"

/**
 *
 */
class GaussianMisfit : public Material, public ReporterInterface
{
public:
  static InputParameters validParams();

  GaussianMisfit(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Simulation variable
  const VariableValue & _sim_var;
  /// Base name of the material system
  const std::string _base_name;

  /// Misfit value at each quadrature point
  MaterialProperty<Real> & _misfit;

  /// Gradient of misfit with respect to material properties
  MaterialProperty<Real> & _misfit_gradient;

  /// Coupled
  /// bool if data format read in is points
  const bool _read_in_points;

  /// values at each xyz coordinate
  const std::vector<Real> & _measurement_values;
  /// x coordinate
  const std::vector<Real> & _coordx;
  /// y coordinate
  const std::vector<Real> & _coordy;
  ///z coordinate
  const std::vector<Real> & _coordz;
  ///xyz point
  const std::vector<Point> & _measurement_points;

  /// beam width
  const Real _beam_width;

private:
  Real computeGuassian(const Point & pt_measured, const Point & pt_sim);

  /// convenience vectors (these are not const because reporters can change their size)
  std::vector<Real> _ones_vec;
  std::vector<Real> _zeros_vec;
  std::vector<Point> _zeros_pts;
};
