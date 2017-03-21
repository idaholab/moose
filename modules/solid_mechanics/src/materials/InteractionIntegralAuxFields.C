/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "InteractionIntegralAuxFields.h"

template <>
InputParameters
validParams<InteractionIntegralAuxFields>()
{
  InputParameters params = validParams<Material>();
  addInteractionIntegralAuxFieldsParams(params);
  params.addRequiredParam<UserObjectName>("crack_front_definition",
                                          "The CrackFrontDefinition user object name");
  params.addRequiredParam<unsigned int>(
      "crack_front_point_index",
      "The index of the point on the crack front to calculate auxiliary fields at");
  return params;
}

void
addInteractionIntegralAuxFieldsParams(InputParameters & params)
{
  MooseEnum sif_modes(InteractionIntegralAuxFields::getSIFModesEnum());
  std::vector<MooseEnum> sif_modes_vec(1, sif_modes);
  params.addRequiredParam<std::vector<MooseEnum>>(
      "sif_modes",
      sif_modes_vec,
      "Stress intensity factor to calculate. Choices are: " + sif_modes.getRawNames());
  params.addParam<Real>("poissons_ratio", "Poisson's ratio for the material.");
  params.addParam<Real>("youngs_modulus", "Young's modulus of the material.");
}

std::vector<MooseEnum>
InteractionIntegralAuxFields::getSIFModesVec(unsigned int n)
{
  return std::vector<MooseEnum>(n, InteractionIntegralAuxFields::getSIFModesEnum());
}

MooseEnum
InteractionIntegralAuxFields::getSIFModesEnum()
{
  return MooseEnum("KI KII KIII T", "KI");
}

InteractionIntegralAuxFields::InteractionIntegralAuxFields(const InputParameters & parameters)
  : Material(parameters),
    _appended_index_name(getParam<std::string>("appended_index_name")),
    _aux_stress_I(declareProperty<ColumnMajorMatrix>("aux_stress_I_" + _appended_index_name)),
    _aux_disp_I(declareProperty<ColumnMajorMatrix>("aux_disp_I_" + _appended_index_name)),
    _aux_grad_disp_I(declareProperty<ColumnMajorMatrix>("aux_grad_disp_I_" + _appended_index_name)),
    _aux_strain_I(declareProperty<ColumnMajorMatrix>("aux_strain_I_" + _appended_index_name)),
    _aux_stress_II(declareProperty<ColumnMajorMatrix>("aux_stress_II_" + _appended_index_name)),
    _aux_disp_II(declareProperty<ColumnMajorMatrix>("aux_disp_II_" + _appended_index_name)),
    _aux_grad_disp_II(
        declareProperty<ColumnMajorMatrix>("aux_grad_disp_II_" + _appended_index_name)),
    _aux_strain_II(declareProperty<ColumnMajorMatrix>("aux_strain_II_" + _appended_index_name)),
    _aux_stress_III(declareProperty<ColumnMajorMatrix>("aux_stress_III_" + _appended_index_name)),
    _aux_disp_III(declareProperty<ColumnMajorMatrix>("aux_disp_III_" + _appended_index_name)),
    _aux_grad_disp_III(
        declareProperty<ColumnMajorMatrix>("aux_grad_disp_III_" + _appended_index_name)),
    _aux_strain_III(declareProperty<ColumnMajorMatrix>("aux_strain_III_" + _appended_index_name)),
    _aux_stress_T(declareProperty<ColumnMajorMatrix>("aux_stress_T_" + _appended_index_name)),
    _aux_disp_T(declareProperty<ColumnMajorMatrix>("aux_disp_T_" + _appended_index_name)),
    _aux_grad_disp_T(declareProperty<ColumnMajorMatrix>("aux_grad_disp_T_" + _appended_index_name)),
    _aux_strain_T(declareProperty<ColumnMajorMatrix>("aux_strain_T_" + _appended_index_name)),
    _crack_front_definition(&getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _crack_front_point_index(getParam<unsigned int>("crack_front_point_index")),
    _poissons_ratio(getParam<Real>("poissons_ratio")),
    _youngs_modulus(getParam<Real>("youngs_modulus"))
{
  std::vector<MooseEnum> sif_mode_enum = getParam<std::vector<MooseEnum>>("sif_modes");
  for (std::vector<MooseEnum>::iterator it = sif_mode_enum.begin(); it != sif_mode_enum.end(); ++it)
  {
    _sif_mode.push_back(SIF_MODE(int(*it)));
  }
  // plane strain
  _kappa = 3 - 4 * _poissons_ratio;
  _shear_modulus = _youngs_modulus / (2 * (1 + _poissons_ratio));
}

void
InteractionIntegralAuxFields::computeQpProperties()
{

  // Calculate (r,theta) position of qp relative to crack front
  Point p(_q_point[_qp]);
  _crack_front_definition->calculateRThetaToCrackFront(p, _crack_front_point_index, _r, _theta);

  // Calculate auxiliary stress tensor at current qp
  for (std::vector<SIF_MODE>::iterator it = _sif_mode.begin(); it != _sif_mode.end(); ++it)
  {
    if (*it == KI)
      computeAuxFields(
          *it, _aux_stress_I[_qp], _aux_disp_I[_qp], _aux_grad_disp_I[_qp], _aux_strain_I[_qp]);

    else if (*it == KII)
      computeAuxFields(
          *it, _aux_stress_II[_qp], _aux_disp_II[_qp], _aux_grad_disp_II[_qp], _aux_strain_II[_qp]);

    else if (*it == KIII)
      computeAuxFields(*it,
                       _aux_stress_III[_qp],
                       _aux_disp_III[_qp],
                       _aux_grad_disp_III[_qp],
                       _aux_strain_III[_qp]);

    else if (*it == T)
      computeTFields(_aux_stress_T[_qp], _aux_grad_disp_T[_qp]);
  }
}

void
InteractionIntegralAuxFields::computeAuxFields(const SIF_MODE sif_mode,
                                               ColumnMajorMatrix & stress,
                                               ColumnMajorMatrix & disp,
                                               ColumnMajorMatrix & grad_disp,
                                               ColumnMajorMatrix & strain)
{

  RealVectorValue k(0);
  if (sif_mode == KI)
    k(0) = 1;

  else if (sif_mode == KII)
    k(1) = 1;

  else if (sif_mode == KIII)
    k(2) = 1;

  Real t = _theta;
  Real t2 = _theta / 2;
  Real tt2 = 3 * _theta / 2;
  Real st = std::sin(t);
  Real ct = std::cos(t);
  Real st2 = std::sin(t2);
  Real ct2 = std::cos(t2);
  Real stt2 = std::sin(tt2);
  Real ctt2 = std::cos(tt2);
  Real ctsq = std::pow(ct, 2);
  Real ct2sq = std::pow(ct2, 2);
  Real ct2cu = std::pow(ct2, 3);
  Real sqrt2PiR = std::sqrt(2 * libMesh::pi * _r);

  // Calculate auxiliary stress tensor
  Real s11 = 1 / sqrt2PiR * (k(0) * ct2 * (1 - st2 * stt2) - k(1) * st2 * (2 + ct2 * ctt2));
  Real s22 = 1 / sqrt2PiR * (k(0) * ct2 * (1 + st2 * stt2) + k(1) * st2 * ct2 * ctt2);
  Real s12 = 1 / sqrt2PiR * (k(0) * ct2 * st2 * ctt2 + k(1) * ct2 * (1 - st2 * stt2));
  Real s13 = -1 / sqrt2PiR * k(2) * st2;
  Real s23 = 1 / sqrt2PiR * k(2) * ct2;
  // plain stress
  // Real s33 = 0;
  // plain strain
  Real s33 = _poissons_ratio * (s11 + s22);

  stress(0, 0) = s11;
  stress(0, 1) = s12;
  stress(0, 2) = s13;
  stress(1, 0) = s12;
  stress(1, 1) = s22;
  stress(1, 2) = s23;
  stress(2, 0) = s13;
  stress(2, 1) = s23;
  stress(2, 2) = s33;

  // Calculate x1 derivative of auxiliary stress tensor
  Real ds111 =
      k(0) / (2 * _r * sqrt2PiR) * (-ct * ct2 + ct * ct2 * st2 * stt2 + st * st2 - st * stt2 +
                                    2 * st * ct2 * ct2 * stt2 + 3 * st * ct2 * st2 * ctt2) +
      k(1) / (2 * _r * sqrt2PiR) *
          (2 * st2 * ct + ct * st2 * ct2 * ctt2 + 2 * st * ct2 - st * ctt2 +
           2 * st * ct2 * ct2 * ctt2 - 3 * st * st2 * ct2 * stt2);
  Real ds121 =
      k(0) / (2 * _r * sqrt2PiR) * (-ct * ct2 * st2 * ctt2 + st * ctt2 - 2 * st * ct2 * ct2 * ctt2 +
                                    3 * st * st2 * ct2 * stt2) +
      k(1) / (2 * _r * sqrt2PiR) * (-ct * ct2 + ct * ct2 * st2 * stt2 + st * st2 - st * stt2 +
                                    2 * st * ct2 * ct2 * stt2 + 3 * st * ct2 * st2 * ctt2);
  Real ds131 = k(2) / (2 * _r * sqrt2PiR) * (st2 * ct + ct2 * st);
  Real ds221 =
      k(0) / (2 * _r * sqrt2PiR) * (-ct * ct2 - ct * ct2 * st2 * stt2 + st * st2 + st * stt2 -
                                    2 * st * ct2 * ct2 * stt2 - 3 * st * ct2 * st2 * ctt2) +
      k(1) / (2 * _r * sqrt2PiR) * (-ct * ct2 * st2 * ctt2 + st * ctt2 - 2 * st * ct2 * ct2 * ctt2 +
                                    3 * st * st2 * ct2 * stt2);
  Real ds231 = k(2) / (2 * _r * sqrt2PiR) * (-ct2 * ct + st2 * st);
  Real ds331 = _poissons_ratio * (ds111 + ds221);

  // Calculate auxiliary displacements
  disp(0, 0) =
      1 / (2 * _shear_modulus) * std::sqrt(_r / (2 * libMesh::pi)) *
      (k(0) * ct2 * (_kappa - 1 + 2 * st2 * st2) + k(1) * st2 * (_kappa + 1 + 2 * ct2 * ct2));
  disp(0, 1) =
      1 / (2 * _shear_modulus) * std::sqrt(_r / (2 * libMesh::pi)) *
      (k(0) * st2 * (_kappa + 1 - 2 * ct2 * ct2) - k(1) * ct2 * (_kappa - 1 - 2 * st2 * st2));
  disp(0, 2) = 1 / _shear_modulus * std::sqrt(_r / (2 * libMesh::pi)) * k(2) * st2 * st2;

  // Calculate x1 derivative of auxiliary displacements
  Real du11 = k(0) / (4 * _shear_modulus * sqrt2PiR) *
                  (ct * ct2 * _kappa + ct * ct2 - 2 * ct * ct2cu + st * st2 * _kappa + st * st2 -
                   6 * st * st2 * ct2sq) +
              k(1) / (4 * _shear_modulus * sqrt2PiR) *
                  (ct * st2 * _kappa + ct * st2 + 2 * ct * st2 * ct2sq - st * ct2 * _kappa +
                   3 * st * ct2 - 6 * st * ct2cu);

  Real du21 = k(0) / (4 * _shear_modulus * sqrt2PiR) *
                  (ct * st2 * _kappa + ct * st2 - 2 * ct * st2 * ct2sq - st * ct2 * _kappa -
                   5 * st * ct2 + 6 * st * ct2cu) +
              k(1) / (4 * _shear_modulus * sqrt2PiR) *
                  (-ct * ct2 * _kappa + 3 * ct * ct2 - 2 * ct * ct2cu - st * st2 * _kappa +
                   3 * st * st2 - 6 * st * st2 * ct2sq);

  Real du31 = k(2) / (_shear_modulus * sqrt2PiR) * (st2 * ct - ct2 * st);

  grad_disp(0, 0) = du11;
  grad_disp(0, 1) = du21;
  grad_disp(0, 2) = du31;

  // Calculate second derivatives of displacements (u,1i)
  // only needed for inhomogenous materials
  Real du111 = k(0) / (8 * _shear_modulus * _r * sqrt2PiR) *
                   (-2 * ctsq * ct2 * _kappa + ct2 * _kappa + 10 * ctsq * ct2 - 11 * ct2 -
                    12 * ctsq * ct2cu + 14 * ct2cu - 2 * ct * st2 * st * _kappa -
                    2 * ct * st2 * st + 12 * ct * st2 * st * ct2sq) +
               k(1) / (8 * _shear_modulus * _r * sqrt2PiR) *
                   (-2 * ctsq * st2 * _kappa - 6 * ctsq * st2 + 12 * ctsq * st2 * ct2sq +
                    2 * ct * ct2 * st * _kappa - 6 * ct * ct2 * st + 12 * ct * ct2cu * st +
                    st2 * _kappa + 5 * st2 - 14 * st2 * ct2sq);
  Real du112 = k(0) / (8 * _shear_modulus * _r * sqrt2PiR) *
                   (-2 * ct * ct2 * st * _kappa + 10 * ct * ct2 * st - 12 * st * ct * ct2cu -
                    st2 * _kappa + 2 * ctsq * st2 * _kappa - st2 + 2 * ctsq * st2 +
                    6 * st2 * ct2sq - 12 * ctsq * st2 * ct2sq) +
               k(2) / (8 * _shear_modulus * _r * sqrt2PiR) *
                   (-2 * ct * st2 * st * _kappa - 6 * ct * st2 * st + 12 * ct * st2 * st * ct2sq +
                    ct2 * _kappa + 6 * ctsq * ct2 - 2 * ctsq * ct2 * _kappa - 3 * ct2 + 6 * ct2cu -
                    12 * ctsq * ct2cu);
  Real du113 = 0.0;

  Real du211 = k(0) / (8 * _shear_modulus * _r * sqrt2PiR) *
                   (-2 * ctsq * ct2 * _kappa + ct2 * _kappa + 10 * ctsq * ct2 - 11 * ct2 -
                    12 * ctsq * ct2cu + 14 * ct2cu - 2 * ct * st2 * st * _kappa -
                    2 * ct * st2 * st + 12 * ct * st2 * st * ct2sq) +
               k(1) / (8 * _shear_modulus * _r * sqrt2PiR) *
                   (-2 * ctsq * st2 * _kappa - 6 * ctsq * st2 + 12 * ctsq * st2 * ct2sq +
                    2 * ct * ct2 * st * _kappa - 6 * ct * ct2 * st + 12 * ct * ct2cu * st +
                    st2 * _kappa + 5 * st2 - 14 * st2 * ct2sq);

  Real du212 = k(0) / (8 * _shear_modulus * _r * sqrt2PiR) *
                   (-2 * ct * st2 * st * _kappa + 2 * ct * st2 * st - 6 * ct2cu -
                    12 * ct * st2 * st * ct2sq + ct2 * _kappa - 2 * ctsq * ct2 * _kappa + 5 * ct2 -
                    10 * ctsq * ct2 + 12 * ctsq * ct2cu) +
               k(2) / (8 * _shear_modulus * _r * sqrt2PiR) *
                   (2 * ct * ct2 * st * _kappa + 6 * ct * ct2 * st + st2 * _kappa -
                    2 * ctsq * st2 * _kappa - 3 * st2 + 6 * ctsq * st2 + 6 * st2 * ct2sq -
                    12 * ctsq * st2 * ct2sq - 12 * ct * ct2cu * st);

  Real du213 = 0.0;

  Real du311 =
      k(2) / (2 * _shear_modulus * _r * sqrt2PiR) * (-2 * ctsq * st2 + st2 + 2 * ct * ct2 * st);

  Real du312 =
      k(2) / (2 * _shear_modulus * _r * sqrt2PiR) * (-2 * ct * st2 * st + ct2 - 2 * ctsq * ct2);

  Real du313 = 0.0;

  // Calculate auxiliary strains
  strain(0, 0) = (1 / _youngs_modulus) * (s11 - _poissons_ratio * (s22 + s33));
  strain(1, 1) = (1 / _youngs_modulus) * (s22 - _poissons_ratio * (s11 + s33));
  strain(2, 2) = (1 / _youngs_modulus) * (s33 - _poissons_ratio * (s11 + s22));
  strain(0, 1) = (1 / _shear_modulus) * s12;
  strain(1, 0) = (1 / _shear_modulus) * s12;
  strain(1, 2) = (1 / _shear_modulus) * s23;
  strain(2, 1) = (1 / _shear_modulus) * s23;
  strain(0, 2) = (1 / _shear_modulus) * s13;
  strain(2, 0) = (1 / _shear_modulus) * s13;

  // Compiling in debug mode says all these variables are unused. I'm
  // ignoring them to silence compiler warnings, but I wonder why are
  // you calculating them at all?
  libmesh_ignore(ds121);
  libmesh_ignore(ds131);
  libmesh_ignore(ds231);
  libmesh_ignore(ds331);

  libmesh_ignore(du111);
  libmesh_ignore(du112);
  libmesh_ignore(du113);
  libmesh_ignore(du211);
  libmesh_ignore(du212);
  libmesh_ignore(du213);
  libmesh_ignore(du311);
  libmesh_ignore(du312);
  libmesh_ignore(du313);
}

void
InteractionIntegralAuxFields::computeTFields(ColumnMajorMatrix & stress,
                                             ColumnMajorMatrix & grad_disp)
{

  Real t = _theta;
  Real st = std::sin(t);
  Real ct = std::cos(t);
  Real stsq = std::pow(st, 2);
  Real ctsq = std::pow(ct, 2);
  Real ctcu = std::pow(ct, 3);
  Real oneOverPiR = 1.0 / (libMesh::pi * _r);

  stress(0, 0) = -oneOverPiR * ctcu;
  stress(0, 1) = -oneOverPiR * st * ctsq;
  stress(0, 2) = 0.0;
  stress(1, 0) = -oneOverPiR * st * ctsq;
  stress(1, 1) = -oneOverPiR * ct * stsq;
  stress(1, 2) = 0.0;
  stress(2, 0) = 0.0;
  stress(2, 1) = 0.0;
  stress(2, 2) = -oneOverPiR * _poissons_ratio * (ctcu + ct * stsq);

  grad_disp(0, 0) = oneOverPiR / (4 * _youngs_modulus) *
                    (ct * (4 * std::pow(_poissons_ratio, 2) - 3 + _poissons_ratio) -
                     std::cos(3 * t) * (1 + _poissons_ratio));
  grad_disp(0, 1) = -oneOverPiR / (4 * _youngs_modulus) *
                    (st * (4 * std::pow(_poissons_ratio, 2) - 3 + _poissons_ratio) +
                     std::sin(3 * t) * (1 + _poissons_ratio));
  grad_disp(0, 2) = 0.0;
}
