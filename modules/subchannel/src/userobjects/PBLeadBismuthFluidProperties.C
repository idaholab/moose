/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

// Lead, LBE, Bismuth properties from V. Sobolev, "Database of thermophysical properties of liquid
// metal coolants for GEN-IV," 2011.
#include "PBLeadBismuthFluidProperties.h"

registerMooseObject("SubChannelApp", PBLeadBismuthFluidProperties);
// Use the array to initialize the const static vector
const Real lead_T[] = {
    600.6,  610.6,  620.6,  630.6,  640.6,  650.6,  660.6,  670.6,  680.6,  690.6,  700.6,  710.6,
    720.6,  730.6,  740.6,  750.6,  760.6,  770.6,  780.6,  790.6,  800.6,  810.6,  820.6,  830.6,
    840.6,  850.6,  860.6,  870.6,  880.6,  890.6,  900.6,  910.6,  920.6,  930.6,  940.6,  950.6,
    960.6,  970.6,  980.6,  990.6,  1000.6, 1010.6, 1020.6, 1030.6, 1040.6, 1050.6, 1060.6, 1070.6,
    1080.6, 1090.6, 1100.6, 1110.6, 1120.6, 1130.6, 1140.6, 1150.6, 1160.6, 1170.6, 1180.6, 1190.6,
    1200.6, 1210.6, 1220.6, 1230.6, 1240.6, 1250.6, 1260.6, 1270.6, 1280.6, 1290.6, 1300.6, 1310.6,
    1320.6, 1330.6, 1340.6, 1350.6, 1360.6, 1370.6, 1380.6, 1390.6, 1400.6, 1410.6, 1420.6, 1430.6,
    1440.6, 1450.6, 1460.6, 1470.6, 1480.6, 1490.6, 1500.6, 1510.6, 1520.6, 1530.6, 1540.6, 1550.6,
    1560.6, 1570.6, 1580.6, 1590.6, 1600.6, 1610.6, 1620.6, 1630.6, 1640.6, 1650.6, 1660.6, 1670.6,
    1680.6, 1690.6, 1700.6, 1710.6, 1720.6, 1730.6, 1740.6, 1750.6, 1760.6, 1770.6, 1780.6, 1790.6,
    1800.6, 1810.6, 1820.6, 1830.6, 1840.6, 1850.6, 1860.6, 1870.6, 1880.6, 1890.6, 1900.6, 1910.6,
    1920.6, 1930.6, 1940.6, 1950.6, 1960.6, 1970.6, 1980.6, 1990.6, 2000.6, 2010.6, 2020.6};
// lead temperature vector corresponding to _e_lead_vec enthalpy vector
const std::vector<Real> PBLeadBismuthFluidProperties::_temperature_lead_vec(
    lead_T, lead_T + sizeof(lead_T) / sizeof(lead_T[0]));

const Real lead_e[] = {
    8.88750E+04, 9.03570E+04, 9.18373E+04, 9.33159E+04, 9.47930E+04, 9.62683E+04, 9.77419E+04,
    9.92139E+04, 1.00684E+05, 1.02152E+05, 1.03619E+05, 1.05084E+05, 1.06547E+05, 1.08009E+05,
    1.09468E+05, 1.10926E+05, 1.12382E+05, 1.13837E+05, 1.15289E+05, 1.16740E+05, 1.18189E+05,
    1.19637E+05, 1.21082E+05, 1.22526E+05, 1.23969E+05, 1.25409E+05, 1.26848E+05, 1.28285E+05,
    1.29721E+05, 1.31155E+05, 1.32587E+05, 1.34018E+05, 1.35448E+05, 1.36875E+05, 1.38302E+05,
    1.39726E+05, 1.41150E+05, 1.42572E+05, 1.43992E+05, 1.45411E+05, 1.46828E+05, 1.48245E+05,
    1.49660E+05, 1.51073E+05, 1.52485E+05, 1.53896E+05, 1.55306E+05, 1.56715E+05, 1.58122E+05,
    1.59528E+05, 1.60933E+05, 1.62337E+05, 1.63740E+05, 1.65141E+05, 1.66542E+05, 1.67942E+05,
    1.69341E+05, 1.70738E+05, 1.72135E+05, 1.73531E+05, 1.74926E+05, 1.76320E+05, 1.77714E+05,
    1.79106E+05, 1.80498E+05, 1.81889E+05, 1.83280E+05, 1.84669E+05, 1.86058E+05, 1.87447E+05,
    1.88835E+05, 1.90222E+05, 1.91609E+05, 1.92995E+05, 1.94381E+05, 1.95766E+05, 1.97151E+05,
    1.98536E+05, 1.99920E+05, 2.01304E+05, 2.02688E+05, 2.04071E+05, 2.05454E+05, 2.06837E+05,
    2.08220E+05, 2.09603E+05, 2.10985E+05, 2.12368E+05, 2.13750E+05, 2.15133E+05, 2.16515E+05,
    2.17897E+05, 2.19280E+05, 2.20663E+05, 2.22045E+05, 2.23428E+05, 2.24811E+05, 2.26195E+05,
    2.27578E+05, 2.28962E+05, 2.30347E+05, 2.31731E+05, 2.33116E+05, 2.34502E+05, 2.35887E+05,
    2.37274E+05, 2.38661E+05, 2.40048E+05, 2.41436E+05, 2.42824E+05, 2.44214E+05, 2.45603E+05,
    2.46994E+05, 2.48385E+05, 2.49777E+05, 2.51170E+05, 2.52564E+05, 2.53958E+05, 2.55354E+05,
    2.56750E+05, 2.58147E+05, 2.59546E+05, 2.60945E+05, 2.62345E+05, 2.63746E+05, 2.65149E+05,
    2.66553E+05, 2.67957E+05, 2.69363E+05, 2.70771E+05, 2.72179E+05, 2.73589E+05, 2.75000E+05,
    2.76413E+05, 2.77826E+05, 2.79242E+05, 2.80659E+05, 2.82077E+05, 2.83497E+05, 2.84918E+05,
    2.86341E+05, 2.87765E+05, 2.89192E+05};
// lead enthalpy vector corresponding to _temperature_lead_vec temperature vector
const std::vector<Real>
    PBLeadBismuthFluidProperties::_e_lead_vec(lead_e, lead_e + sizeof(lead_e) / sizeof(lead_e[0]));

const Real bismuth_T[] = {
    544.6,  554.6,  564.6,  574.6,  584.6,  594.6,  604.6,  614.6,  624.6,  634.6,  644.6,  654.6,
    664.6,  674.6,  684.6,  694.6,  704.6,  714.6,  724.6,  734.6,  744.6,  754.6,  764.6,  774.6,
    784.6,  794.6,  804.6,  814.6,  824.6,  834.6,  844.6,  854.6,  864.6,  874.6,  884.6,  894.6,
    904.6,  914.6,  924.6,  934.6,  944.6,  954.6,  964.6,  974.6,  984.6,  994.6,  1004.6, 1014.6,
    1024.6, 1034.6, 1044.6, 1054.6, 1064.6, 1074.6, 1084.6, 1094.6, 1104.6, 1114.6, 1124.6, 1134.6,
    1144.6, 1154.6, 1164.6, 1174.6, 1184.6, 1194.6, 1204.6, 1214.6, 1224.6, 1234.6, 1244.6, 1254.6,
    1264.6, 1274.6, 1284.6, 1294.6, 1304.6, 1314.6, 1324.6, 1334.6, 1344.6, 1354.6, 1364.6, 1374.6,
    1384.6, 1394.6, 1404.6, 1414.6, 1424.6, 1434.6, 1444.6, 1454.6, 1464.6, 1474.6, 1484.6, 1494.6,
    1504.6, 1514.6, 1524.6, 1534.6, 1544.6, 1554.6, 1564.6, 1574.6, 1584.6, 1594.6, 1604.6, 1614.6,
    1624.6, 1634.6, 1644.6, 1654.6, 1664.6, 1674.6, 1684.6, 1694.6, 1704.6, 1714.6, 1724.6, 1734.6,
    1744.6, 1754.6, 1764.6, 1774.6, 1784.6, 1794.6, 1804.6, 1814.6, 1824.6};
// lead temperature vector corresponding to _e_bismuth_vec enthalpy vector
const std::vector<Real> PBLeadBismuthFluidProperties::_temperature_bismuth_vec(
    bismuth_T, bismuth_T + sizeof(bismuth_T) / sizeof(bismuth_T[0]));

const Real bismuth_e[] = {
    7.93212E+04, 8.07736E+04, 8.22182E+04, 8.36554E+04, 8.50857E+04, 8.65093E+04, 8.79267E+04,
    8.93382E+04, 9.07440E+04, 9.21446E+04, 9.35402E+04, 9.49309E+04, 9.63172E+04, 9.76991E+04,
    9.90770E+04, 1.00451E+05, 1.01821E+05, 1.03188E+05, 1.04551E+05, 1.05912E+05, 1.07269E+05,
    1.08623E+05, 1.09975E+05, 1.11324E+05, 1.12670E+05, 1.14014E+05, 1.15356E+05, 1.16696E+05,
    1.18033E+05, 1.19369E+05, 1.20703E+05, 1.22035E+05, 1.23365E+05, 1.24693E+05, 1.26020E+05,
    1.27346E+05, 1.28670E+05, 1.29993E+05, 1.31314E+05, 1.32635E+05, 1.33954E+05, 1.35272E+05,
    1.36589E+05, 1.37905E+05, 1.39220E+05, 1.40534E+05, 1.41847E+05, 1.43159E+05, 1.44471E+05,
    1.45782E+05, 1.47092E+05, 1.48401E+05, 1.49710E+05, 1.51019E+05, 1.52326E+05, 1.53633E+05,
    1.54940E+05, 1.56246E+05, 1.57552E+05, 1.58857E+05, 1.60162E+05, 1.61467E+05, 1.62771E+05,
    1.64075E+05, 1.65379E+05, 1.66682E+05, 1.67985E+05, 1.69288E+05, 1.70591E+05, 1.71893E+05,
    1.73195E+05, 1.74497E+05, 1.75800E+05, 1.77101E+05, 1.78403E+05, 1.79705E+05, 1.81007E+05,
    1.82308E+05, 1.83610E+05, 1.84911E+05, 1.86213E+05, 1.87514E+05, 1.88816E+05, 1.90117E+05,
    1.91419E+05, 1.92721E+05, 1.94022E+05, 1.95324E+05, 1.96626E+05, 1.97928E+05, 1.99230E+05,
    2.00532E+05, 2.01835E+05, 2.03137E+05, 2.04440E+05, 2.05742E+05, 2.07045E+05, 2.08349E+05,
    2.09652E+05, 2.10955E+05, 2.12259E+05, 2.13563E+05, 2.14867E+05, 2.16171E+05, 2.17476E+05,
    2.18780E+05, 2.20085E+05, 2.21391E+05, 2.22696E+05, 2.24002E+05, 2.25308E+05, 2.26614E+05,
    2.27921E+05, 2.29228E+05, 2.30535E+05, 2.31842E+05, 2.33150E+05, 2.34458E+05, 2.35766E+05,
    2.37075E+05, 2.38384E+05, 2.39693E+05, 2.41003E+05, 2.42313E+05, 2.43623E+05, 2.44934E+05,
    2.46245E+05, 2.47556E+05, 2.48868E+05};
// lead enthalpy vector corresponding to _temperature_bismuth_vec temperature vector
const std::vector<Real> PBLeadBismuthFluidProperties::_e_bismuth_vec(
    bismuth_e, bismuth_e + sizeof(bismuth_e) / sizeof(bismuth_e[0]));

// Use the array to initialize the const static vector
const Real lbe_T[] = {
    398,  408,  418,  428,  438,  448,  458,  468,  478,  488,  498,  508,  518,  528,  538,  548,
    558,  568,  578,  588,  598,  608,  618,  628,  638,  648,  658,  668,  678,  688,  698,  708,
    718,  728,  738,  748,  758,  768,  778,  788,  798,  808,  818,  828,  838,  848,  858,  868,
    878,  888,  898,  908,  918,  928,  938,  948,  958,  968,  978,  988,  998,  1008, 1018, 1028,
    1038, 1048, 1058, 1068, 1078, 1088, 1098, 1108, 1118, 1128, 1138, 1148, 1158, 1168, 1178, 1188,
    1198, 1208, 1218, 1228, 1238, 1248, 1258, 1268, 1278, 1288, 1298, 1308, 1318, 1328, 1338, 1348,
    1358, 1368, 1378, 1388, 1398, 1408, 1418, 1428, 1438, 1448, 1458, 1468, 1478, 1488, 1498, 1508,
    1518, 1528, 1538, 1548, 1558, 1568, 1578, 1588, 1598, 1608, 1618, 1628, 1638, 1648, 1658, 1668,
    1678, 1688, 1698, 1708, 1718, 1728, 1738, 1748, 1758, 1768, 1778, 1788, 1798, 1808, 1818, 1828,
    1838, 1848, 1858, 1868, 1878, 1888, 1898, 1908, 1918};
// lead temperature vector corresponding to _e_lbe_vec enthalpy vector
const std::vector<Real>
    PBLeadBismuthFluidProperties::_temperature_lbe_vec(lbe_T,
                                                       lbe_T + sizeof(lbe_T) / sizeof(lbe_T[0]));

const Real lbe_e[] = {
    5.89916E+04, 6.04731E+04, 6.19529E+04, 6.34311E+04, 6.49076E+04, 6.63824E+04, 6.78554E+04,
    6.93265E+04, 7.07957E+04, 7.22630E+04, 7.37284E+04, 7.51918E+04, 7.66532E+04, 7.81127E+04,
    7.95702E+04, 8.10256E+04, 8.24790E+04, 8.39305E+04, 8.53799E+04, 8.68272E+04, 8.82726E+04,
    8.97159E+04, 9.11572E+04, 9.25965E+04, 9.40338E+04, 9.54692E+04, 9.69025E+04, 9.83338E+04,
    9.97632E+04, 1.01191E+05, 1.02616E+05, 1.04040E+05, 1.05461E+05, 1.06881E+05, 1.08299E+05,
    1.09715E+05, 1.11129E+05, 1.12542E+05, 1.13952E+05, 1.15361E+05, 1.16768E+05, 1.18173E+05,
    1.19576E+05, 1.20978E+05, 1.22378E+05, 1.23776E+05, 1.25173E+05, 1.26568E+05, 1.27961E+05,
    1.29353E+05, 1.30743E+05, 1.32131E+05, 1.33518E+05, 1.34904E+05, 1.36288E+05, 1.37670E+05,
    1.39051E+05, 1.40431E+05, 1.41809E+05, 1.43186E+05, 1.44561E+05, 1.45935E+05, 1.47308E+05,
    1.48680E+05, 1.50050E+05, 1.51419E+05, 1.52786E+05, 1.54153E+05, 1.55518E+05, 1.56882E+05,
    1.58245E+05, 1.59606E+05, 1.60967E+05, 1.62327E+05, 1.63685E+05, 1.65043E+05, 1.66399E+05,
    1.67755E+05, 1.69109E+05, 1.70463E+05, 1.71815E+05, 1.73167E+05, 1.74518E+05, 1.75868E+05,
    1.77218E+05, 1.78566E+05, 1.79914E+05, 1.81261E+05, 1.82607E+05, 1.83952E+05, 1.85297E+05,
    1.86641E+05, 1.87985E+05, 1.89328E+05, 1.90670E+05, 1.92012E+05, 1.93353E+05, 1.94694E+05,
    1.96034E+05, 1.97374E+05, 1.98713E+05, 2.00052E+05, 2.01391E+05, 2.02729E+05, 2.04067E+05,
    2.05405E+05, 2.06742E+05, 2.08079E+05, 2.09416E+05, 2.10752E+05, 2.12089E+05, 2.13425E+05,
    2.14761E+05, 2.16097E+05, 2.17433E+05, 2.18769E+05, 2.20104E+05, 2.21440E+05, 2.22776E+05,
    2.24111E+05, 2.25447E+05, 2.26783E+05, 2.28119E+05, 2.29455E+05, 2.30791E+05, 2.32128E+05,
    2.33464E+05, 2.34801E+05, 2.36138E+05, 2.37476E+05, 2.38813E+05, 2.40151E+05, 2.41490E+05,
    2.42829E+05, 2.44168E+05, 2.45507E+05, 2.46847E+05, 2.48188E+05, 2.49529E+05, 2.50870E+05,
    2.52212E+05, 2.53555E+05, 2.54898E+05, 2.56242E+05, 2.57586E+05, 2.58931E+05, 2.60277E+05,
    2.61624E+05, 2.62971E+05, 2.64319E+05, 2.65668E+05, 2.67018E+05, 2.68368E+05};
// lbe enthalpy vector corresponding to _temperature_lbe_vec temperature vector
const std::vector<Real>
    PBLeadBismuthFluidProperties::_e_lbe_vec(lbe_e, lbe_e + sizeof(lbe_e) / sizeof(lbe_e[0]));

InputParameters
PBLeadBismuthFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  MooseEnum MetalType("Lead Bismuth LBE", "Lead");
  params.addParam<MooseEnum>(
      "metal_type", MetalType, "Metal type options: Lead (default), Bismuth, LBE.");
  params.addParam<Real>("p_0", 1.e5, "Reference pressure");
  return params;
}

PBLeadBismuthFluidProperties::PBLeadBismuthFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters),
    metal_type(getParam<MooseEnum>("metal_type")),
    _p_0(getParam<Real>("p_0"))
{
  switch (metal_type)
  {
    case 0: // Lead
      _Tm = 600.6;
      _Tb = 2021;

      _C0_rho = 11441;
      _C1_rho = -1.2795;

      _C0_k = 9.2;
      _C1_k = 0.011;
      _C2_k = 0;

      _C0_mu = 4.55e-4;
      _C1_mu = 1069;

      _C0_cp = 176.2;
      _C1_cp = -4.923e-2;
      _C2_cp = 1.544e-5;
      _C3_cp = -1.524e6;

      _C0_h = 176.2;
      _C1_h = -2.4615e-2;
      _C2_h = 5.417e-6;
      _C3_h = 1.524e6;

      _Cp_Tmax = cp_from_p_T(_p_0, _Tb);
      _Cp_Tmin = cp_from_p_T(_p_0, _Tm);
      _H_Tmax = h_from_p_T(_p_0, _Tb);
      _H_Tmin = h_from_p_T(_p_0, _Tm);
      break;

    case 1: // Bismuth
      _Tm = 544.6;
      _Tb = 1831;

      _C0_rho = 10725;
      _C1_rho = -1.22;

      _C0_k = 7.34;
      _C1_k = 9.5e-3;
      _C2_k = 0;

      _C0_mu = 4.456e-4;
      _C1_mu = 780;

      _C0_cp = 118.2;
      _C1_cp = 5.934e-3;
      _C2_cp = 0;
      _C3_cp = 7.183e6;

      _C0_h = 118.2;
      _C1_h = 2.967e-3;
      _C2_h = 0;
      _C3_h = -7.183e6;

      _Cp_Tmax = cp_from_p_T(_p_0, _Tb);
      _Cp_Tmin = cp_from_p_T(_p_0, _Tm);
      _H_Tmax = h_from_p_T(_p_0, _Tb);
      _H_Tmin = h_from_p_T(_p_0, _Tm);
      break;

    case 2: // LBE
      _Tm = 398;
      _Tb = 1927;

      _C0_rho = 11065;
      _C1_rho = -1.293;

      _C0_k = 3.284;
      _C1_k = 1.617e-2;
      _C2_k = -2.305e-6;

      _C0_mu = 4.94e-4;
      _C1_mu = 754.1;

      _C0_cp = 164.8;
      _C1_cp = -3.94e-2;
      _C2_cp = 1.25e-5;
      _C3_cp = -4.56e5;

      _C0_h = 164.8;
      _C1_h = -1.97e-2;
      _C2_h = 4.167e-6;
      _C3_h = 4.56e5;

      _Cp_Tmax = cp_from_p_T(_p_0, _Tb);
      _Cp_Tmin = cp_from_p_T(_p_0, _Tm);
      _H_Tmax = h_from_p_T(_p_0, _Tb);
      _H_Tmin = h_from_p_T(_p_0, _Tm);
      break;

    default:
      break;
  }
  _H0 = cp_from_p_T(_p_0, _Tm) * _Tm;
}

Real
PBLeadBismuthFluidProperties::rho_from_p_T(Real /*pressure*/, Real temperature) const
{
  return (_C0_rho + _C1_rho * temperature);
}

void
PBLeadBismuthFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(pressure, temperature);
  drho_dp = 0.;
  drho_dT = _C1_rho;
}

Real
PBLeadBismuthFluidProperties::beta_from_p_T(Real /*pressure*/, Real temperature) const
{
  return -1 / (_C0_rho / _C1_rho + temperature);
}

Real PBLeadBismuthFluidProperties::tm_from_p_T(Real /*pressure*/, Real /*temperature*/) const
{
  return _Tm;
}

Real PBLeadBismuthFluidProperties::dH_from_p_T(Real /*pressure*/, Real /*temperature*/) const
{
  return _dH;
}

Real
PBLeadBismuthFluidProperties::cv_from_p_T(Real /*pressure*/, Real temperature) const
{
  return cp_from_p_T(1e5, temperature);
}

Real
PBLeadBismuthFluidProperties::cp_from_p_T(Real /*pressure*/, Real temperature) const
{
  return (_C0_cp + _C1_cp * temperature + _C2_cp * pow(temperature, 2) +
          _C3_cp * pow(temperature, -2));
}

void
PBLeadBismuthFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(pressure, temperature);
  dcp_dp = 0;
  if (temperature < _Tb && temperature > _Tm)
    dcp_dT = _C1_cp; // From SAM, need to change later
  else
    dcp_dT = 0;
}

Real
PBLeadBismuthFluidProperties::h_from_p_T(Real /*pressure*/, Real temperature) const
{
  if (temperature > _Tb + 1.e-3)
    return _H_Tmax + _Cp_Tmax * (temperature - _Tb);
  else if (temperature < _Tm - 1.e-3)
    return _H_Tmin + _Cp_Tmin * (temperature - _Tm);
  return _C0_h * (temperature - _Tm) + _C1_h * (pow(temperature, 2) - pow(_Tm, 2)) +
         _C2_h * (pow(temperature, 3) - pow(_Tm, 3)) +
         _C3_h * (pow(temperature, -1) - pow(_Tm, -1)) + _H0;
}

Real
PBLeadBismuthFluidProperties::T_from_p_h(Real temperature, Real enthalpy) const
{
  if (enthalpy > _H_Tmax)
  {
    temperature = (enthalpy - _H_Tmax) / _Cp_Tmax + _Tb;
  }
  else if (enthalpy < _H_Tmin)
  {
    temperature = (enthalpy - _H_Tmin) / _Cp_Tmin + _Tm;
  }
  else
  {
    switch (metal_type)
    {
      case 0: // Lead
        for (unsigned int i = 0; i < _e_lead_vec.size() - 1; i++)
        {
          if (enthalpy > _e_lead_vec[i] && enthalpy <= _e_lead_vec[i + 1])
          {
            temperature = _temperature_lead_vec[i] +
                          (enthalpy - _e_lead_vec[i]) / (_e_lead_vec[i + 1] - _e_lead_vec[i]) *
                              (_temperature_lead_vec[i + 1] - _temperature_lead_vec[i]);
            break;
          }
        }
        break;

      case 1: // Bismuth
        for (unsigned int i = 0; i < _e_bismuth_vec.size() - 1; i++)
        {
          if (enthalpy > _e_bismuth_vec[i] && enthalpy <= _e_bismuth_vec[i + 1])
          {
            temperature = _temperature_bismuth_vec[i] +
                          (enthalpy - _e_bismuth_vec[i]) /
                              (_e_bismuth_vec[i + 1] - _e_bismuth_vec[i]) *
                              (_temperature_bismuth_vec[i + 1] - _temperature_bismuth_vec[i]);
            break;
          }
        }
        break;

      case 2: // LBE
        for (unsigned int i = 0; i < _e_lbe_vec.size() - 1; i++)
        {
          if (enthalpy > _e_lbe_vec[i] && enthalpy <= _e_lbe_vec[i + 1])
          {
            temperature = _temperature_lbe_vec[i] +
                          (enthalpy - _e_lbe_vec[i]) / (_e_lbe_vec[i + 1] - _e_lbe_vec[i]) *
                              (_temperature_lbe_vec[i + 1] - _temperature_lbe_vec[i]);
            break;
          }
        }
        break;

      default:
        break;
    }
  }
  return temperature;
}

Real
PBLeadBismuthFluidProperties::mu_from_p_T(Real /*pressure*/, Real temperature) const
{
  Real mu = _C0_mu * exp(_C1_mu / temperature);
  return mu;
}

Real
PBLeadBismuthFluidProperties::k_from_p_T(Real /*pressure*/, Real temperature) const
{
  return (_C0_k + _C1_k * temperature + _C2_k * pow(temperature, 2));
}

Real
PBLeadBismuthFluidProperties::temperature_correction(Real & temperature) const
{
  if (temperature > _Tb)
    return _Tb;
  else if (temperature < _Tm)
    return _Tm;
  else
    return temperature;
}
