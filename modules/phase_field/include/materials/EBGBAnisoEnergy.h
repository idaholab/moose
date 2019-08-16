//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeParsedMaterialHelper.h"
#include "ExpressionBuilder.h"
#include "EulerAngleProvider.h"

// Forward Declarations
class EBGBAnisoEnergy;

template <>
InputParameters validParams<EBGBAnisoEnergy>();

/**
 * Grain boundary energy parameters for isotropic uniform grain boundary energies
 */
class EBGBAnisoEnergy : public DerivativeParsedMaterialHelper<>,
		     public ExpressionBuilder
{
public:
  EBGBAnisoEnergy(const InputParameters & parameters);

protected:
  const unsigned int _op_num;
  const int _thickness;
  const int _sharpness;

	///@{ holds the values for the idealized rotations
  std::vector<std::vector<EBTerm> > _geom100;
  std::vector<std::vector<EBTerm> > _geom110;
  std::vector<std::vector<EBTerm> > _geom111;
  //@}

  std::vector<EBVectorFunction> _axes100; // all of the <100> axes
  std::vector<EBVectorFunction> _dirs100; // orthogonal to the <100> axes
  std::vector<EBVectorFunction> _axes110;
  std::vector<EBVectorFunction> _dirs110;
  std::vector<EBVectorFunction> _axes111;
  std::vector<EBVectorFunction> _dirs111;

  ///{@ 90 degree rotaions around the coordinate axes
  const RotationTensor _rot_X_p90;
  const RotationTensor _rot_Y_p90;
  const RotationTensor _rot_Z_p90;
  const RotationTensor _rot_Z_n90;
  ///@}

  std::vector<EBMatrixFunction> _symmetry_variants;

	const Real _epsilon;

	std::vector<EBMatrixFunction> _orientation_matrix;

	std::vector<EBTerm> _e100, _e110, _e111;
  std::vector<EBTerm> _s100, _s110, _s111;
  std::vector<EBTerm> _ksi, _ksi_back, _en1, _en2;
  std::vector<EBTerm> _entwist, _entilt;

	std::vector<Real> _par_vec;

	std::string _op_to_grain_name;

	enum class Material
  {
    Cu,
    Al,
    Au,
    Ni,
  } _material;

  void setParVec();

	void setAxes100();
	void setAxes110();
	void setAxes111();

  EBTerm gB5DOF(EBMatrixFunction P, EBMatrixFunction S);

	void distancesToSet(const EBMatrixFunction & P, EBMatrixFunction & S,
											std::vector<std::vector<EBTerm> > & geom,
										  const std::vector<EBVectorFunction> & axes,
										  const std::vector<EBVectorFunction> & dirs);

	void trimGeom(std::vector<std::vector<EBTerm>> & geom);

	EBTerm weightedMeanEnergy();

	void set100();
	void set110();
	void set111();

	void twist100(std::vector<EBTerm> ksi);
	void twist110(std::vector<EBTerm> ksi);
	void twist111(std::vector<EBTerm> ksi);

	void aTGB100(const std::vector<EBTerm> & eta, const std::vector<EBTerm> & ksi);
	void aTGB110(const std::vector<EBTerm> & eta, const std::vector<EBTerm> & ksi);
	void aTGB111(const std::vector<EBTerm> & eta, const std::vector<EBTerm> & ksi);

  void sTGB100(const std::vector<EBTerm> & ksi, std::vector<EBTerm> & en);
	void sTGB110(const std::vector<EBTerm> & ksi, std::vector<EBTerm> & en);

	EBTerm rSW(EBTerm theta, Real thetaMin, Real thetaMax, Real a);
};
