"""Run DASSH with the Upgraded Chen-Todreas (UCTD) correlations for EBR-II XX09 and write the data
compared with SCM in the friction factor closures verification page.

DASSH (https://github.com/dassh-dev/dassh) must be installed in the Python environment running this
script. Two CSV files are written next to this script:

1. dassh_XX09_SS17_out.csv: position, type, mass flow rate, and temperature of every subchannel at
   the TTC height for the SHRT-17 steady state in dassh_XX09_SS17.txt.
2. dassh_tri_wire_out.csv: DASSH UCTD friction factor and Reynolds number of the interior, edge,
   and corner subchannels versus the bundle Reynolds number, in the format of tri_wire_out.csv.
"""

import os
import shutil
import tempfile
from pathlib import Path

import numpy as np
import dassh
from dassh.correlations import flowsplit_ctd, flowsplit_uctd

SCRIPT_DIR = Path(__file__).resolve().parent
INPUT = "dassh_XX09_SS17.txt"

# Geometry and axial power shape of XX09_SCM_SS17.i
HEATED_LENGTH = 0.343
LENGTH = 0.612
ALPHA = 1.8012
N_PINS = 61
# Axial regions of constant pin power in the heated length, finer than the SCM axial cells
# (50 cells over 0.612 m, about 28 in the heated length)
N_HEATED_REGIONS = 50

# UCTD transition exponent lambda of Chen et al. (2018), as used by flowsplit_uctd
UCTD_LAMBDA = 7


def axial_heat_rate(z):
    """Axial shape of the pin linear power, the axial_heat_rate function of XX09_SCM_SS17.i"""
    L = HEATED_LENGTH
    return (
        (np.pi / 2)
        * np.sin(np.pi * z / L)
        * np.exp(-ALPHA * z)
        / (1.0 / ALPHA * (1.0 - np.exp(-ALPHA * L)))
        * L
    )


def write_power(path):
    """Write the DASSH user power file: constant pin linear power in each axial region. DASSH
    normalizes the power to total_power, so only the shape matters."""
    z = np.linspace(0.0, HEATED_LENGTH, N_HEATED_REGIONS + 1)
    rows = []
    for z_lo, z_hi in zip(z[:-1], z[1:]):
        # Average of the axial shape over the region
        zq = np.linspace(z_lo, z_hi, 101)
        q = np.trapz(axial_heat_rate(zq), zq) / (z_hi - z_lo)
        rows += [[1, 1, z_lo, z_hi, pin, q] for pin in range(1, N_PINS + 1)]
    # Unheated length above the heated section
    rows += [[1, 1, HEATED_LENGTH, LENGTH, pin, 0.0] for pin in range(1, N_PINS + 1)]
    np.savetxt(
        path, rows, delimiter=",", fmt=["%d", "%d", "%.8e", "%.8e", "%d", "%.8e"]
    )


def run_dassh(wdir):
    """Run the SHRT-17 steady state and return the reactor"""
    shutil.copy(SCRIPT_DIR / INPUT, wdir)
    write_power(os.path.join(wdir, "dassh_XX09_SS17_power.csv"))
    inp = dassh.DASSH_Input(os.path.join(wdir, INPUT))
    reactor = dassh.Reactor(inp, path=wdir, write_output=True)
    reactor.temperature_sweep()
    reactor.postprocess()
    return reactor


def write_profile(reactor, wdir):
    """Write the subchannel mass flow rate and temperature at the TTC height"""
    table = np.genfromtxt(
        os.path.join(wdir, "temp_coolant_subchannel_a=1.csv"), delimiter=",", dtype=str
    )
    # Rows 0 and 1 hold the height and the average temperature; the others hold one subchannel
    x = table[2:, 0].astype(float)
    y = table[2:, 1].astype(float)
    sc_type = table[2:, 2]
    T = table[2:, 3].astype(float)
    # DASSH mass flow rates do not vary axially; same subchannel order as the table
    mdot = reactor.assemblies[0].rodded.sc_mfr
    with open(SCRIPT_DIR / "dassh_XX09_SS17_out.csv", "w") as f:
        f.write("x,y,type,mdot,T\n")
        for row in zip(x, y, sc_type, mdot, T):
            f.write("{:.8e},{:.8e},{},{:.8e},{:.8e}\n".format(*row))


def write_friction_factor(reactor):
    """Write the DASSH UCTD subchannel friction factors versus the bundle Reynolds number.

    DASSH uses the subchannel friction factors in the UCTD flow split and the bundle friction factor
    in the pressure drop. The subchannel friction factors are evaluated with the DASSH flow split and
    the DASSH transition interpolation in flowsplit_ctd._iterate.
    """
    rr = reactor.assemblies[0].rodded
    Cf = rr.corr_constants["ff"]["Cf_sc"]
    Re_bl, Re_bt = rr.corr_constants["ff"]["Re_bnds"]
    de_ratio = rr.params["de"] / rr.bundle_params["de"]
    # Subchannel regime bounds, as in flowsplit_ctd._calc_transition_flowsplit
    Re_iL = Re_bl * de_ratio * rr.corr_constants["fs"]["fs"]["laminar"]
    Re_iT = Re_bt * de_ratio * rr.corr_constants["fs"]["fs"]["turbulent"]
    # Same bundle Reynolds number range as the SCM mass flux sweep in tri_wire.i
    Re_bundle = np.geomspace(1.0, 3.0e5, 400)
    with open(SCRIPT_DIR / "dassh_tri_wire_out.csv", "w") as f:
        f.write("Re_bundle,Re_center,ff_center,Re_edge,ff_edge,Re_corner,ff_corner\n")
        for Re in Re_bundle:
            rr.coolant_int_params["Re"] = Re
            X = flowsplit_uctd.calculate_flow_split(rr)
            Re_i = Re * X * de_ratio
            INT_i = np.clip(np.log10(Re_i / Re_iL) / np.log10(Re_iT / Re_iL), 0.0, 1.0)
            ff = flowsplit_ctd._calc_ffb_tr(
                Cf["laminar"] / Re_i,
                Cf["turbulent"] / Re_i ** flowsplit_ctd._M["turbulent"],
                INT_i,
                flowsplit_ctd._GAMMA,
                UCTD_LAMBDA,
            )
            f.write(
                ",".join(
                    "{:.8e}".format(v)
                    for v in (Re, Re_i[0], ff[0], Re_i[1], ff[1], Re_i[2], ff[2])
                )
                + "\n"
            )


with tempfile.TemporaryDirectory() as wdir:
    reactor = run_dassh(wdir)
    write_profile(reactor, wdir)
    write_friction_factor(reactor)
