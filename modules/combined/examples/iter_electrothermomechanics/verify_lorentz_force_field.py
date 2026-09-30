#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""
Compare MOOSE simulation results to analytical Lorentz force solution.
Comprehensive verification including axisymmetry and axial uniformity checks.

Usage:
  ./verify_lorentz_force_field.py --geometry copper_cylinder
  ./verify_lorentz_force_field.py --geometry cylinder
  ./verify_lorentz_force_field.py --geometry annulus
  ./verify_lorentz_force_field.py --geometry cable
  ./verify_lorentz_force_field.py --data-dir path/to/custom/data
"""

import argparse
import os
import sys

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# Parse command-line arguments
parser = argparse.ArgumentParser(
    description="Verify MOOSE Lorentz force simulation against analytical solution",
    formatter_class=argparse.RawDescriptionHelpFormatter,
    epilog="""
Geometry variants:
  copper_cylinder  : Solid copper (channel + bundle)
  cylinder         : Hybrid copper-Nb3Sn cylinder (channel + bundle)
  annulus          : Hybrid copper-Nb3Sn annulus (bundle only)
  cable            : Full cable with JK2LB steel jacket (channel + bundle + jacket)
    """,
)
parser.add_argument(
    "--geometry",
    type=str,
    default="copper_cylinder",
    choices=["copper_cylinder", "cylinder", "annulus", "cable"],
    help="Geometry variant to verify (default: copper_cylinder)",
)
parser.add_argument(
    "--data-dir",
    type=str,
    default=None,
    help="Custom data directory path (overrides --geometry)",
)
args = parser.parse_args()

# Determine data and figure directories
if args.data_dir is not None:
    data_dir = args.data_dir
    # Extract geometry name from custom path for figures
    geometry_name = os.path.basename(data_dir.rstrip("/"))
else:
    geometry_name = args.geometry
    data_dir = f"data/{geometry_name}"

figures_dir = f"figures/{geometry_name}"

# Create directories if they don't exist
os.makedirs(figures_dir, exist_ok=True)

print(f"Verifying geometry: {geometry_name}")
print(f"Data directory: {data_dir}")
print(f"Figures directory: {figures_dir}")

# Redirect remaining output to log file in data directory
log_file = open(f"{figures_dir}/lorentz_verification.log", "w")
sys.stdout = log_file

# Read physical constants from MOOSE output
csv_filename = f"{data_dir}/{geometry_name}_out.csv"
try:
    constants = pd.read_csv(csv_filename)
    # Get the last timestep values (most recent)
    vacuum_permeability = constants["vacuum_permeability"].iloc[-1]  # N/A^2
    current_density_z = constants["current_density_z"].iloc[-1]  # A/mm^2
    print("Read from MOOSE output:")
    print(f"  vacuum_permeability = {vacuum_permeability:.6e} N/A^2")
    print(f"  current_density_z = {current_density_z:.6e} A/mm^2")
except FileNotFoundError:
    print(f"Warning: Could not find '{csv_filename}'")
    print("Using fallback values. Run the simulation first for accurate constants.")
    vacuum_permeability = 1.25663706e-6  # N/A^2
    current_density_z = 1.0  # A/mm^2


def analytical_lorentz_radial(r):
    """
    Analytical solution for radial Lorentz force per unit volume.
    F_r = -μ₀ * J_z^2 * r / 2

    Parameters
    ----------
    r : array-like
        Radial distance from center (mm)

    Returns
    -------
    F_r : array-like
        Radial Lorentz force per unit volume (N/mm^3), negative indicates compression

    """
    return -vacuum_permeability * current_density_z**2 * r / 2


def process_radial_line(filename, line_name):
    """
    Process one radial line sample.

    Parameters
    ----------
    filename : str
        CSV file to read
    line_name : str
        Descriptive name for this line

    Returns
    -------
    dict : Contains r, lorentz_radial_sim, lorentz_radial_analytical, errors

    """
    try:
        data = pd.read_csv(filename)
    except FileNotFoundError:
        print(f"Error: Could not find '{filename}'")
        return None

    # Extract coordinates and compute radius
    x = data["x"].values
    y = data["y"].values
    r = np.sqrt(x**2 + y**2)

    # Extract Cartesian components from MOOSE simulation
    lorentz_x_sim = data["lorentz_x_aux"].values
    lorentz_y_sim = data["lorentz_y_aux"].values
    lorentz_z_sim = data["lorentz_z_aux"].values

    # Mask for non-zero radius to avoid division by zero
    mask = r > 1e-10

    # Compute radial component from simulation (dot product: F·r̂ = (Fx*x + Fy*y)/r)
    lorentz_radial_sim = np.zeros_like(r)
    lorentz_radial_sim[mask] = (
        lorentz_x_sim[mask] * x[mask] + lorentz_y_sim[mask] * y[mask]
    ) / r[mask]
    print(f"Lorentz force value at r=R: {1e9*lorentz_radial_sim[mask][-1]:.6e} N/m^3")

    # Compute analytical radial solution
    lorentz_radial_analytical = analytical_lorentz_radial(r)

    # Compute errors (only for non-zero radius)
    absolute_error = np.zeros_like(r)
    relative_error = np.zeros_like(r)
    absolute_error[mask] = np.abs(
        lorentz_radial_sim[mask] - lorentz_radial_analytical[mask]
    )
    relative_error[mask] = absolute_error[mask] / np.abs(
        lorentz_radial_analytical[mask]
    )

    return {
        "name": line_name,
        "r": r,
        "mask": mask,
        "lorentz_sim": lorentz_radial_sim,
        "lorentz_analytical": lorentz_radial_analytical,
        "abs_error": absolute_error,
        "rel_error": relative_error,
        "lorentz_z_sim": lorentz_z_sim,
    }


def process_axial_line(filename):
    """
    Process the axial line sample to verify uniformity along z.

    Parameters
    ----------
    filename : str
        CSV file to read

    Returns
    -------
    dict : Contains z, r, lorentz_radial values

    """
    try:
        data = pd.read_csv(filename)
    except FileNotFoundError:
        print(f"Error: Could not find '{filename}'")
        return None

    # Extract coordinates
    x = data["x"].values
    y = data["y"].values
    z = data["z"].values
    r = np.sqrt(x**2 + y**2)

    # Extract Cartesian components
    lorentz_x_sim = data["lorentz_x_aux"].values
    lorentz_y_sim = data["lorentz_y_aux"].values
    lorentz_z_sim = data["lorentz_z_aux"].values

    # Compute radial component
    mask = r > 1e-10
    lorentz_radial_sim = np.zeros_like(r)
    lorentz_radial_sim[mask] = (
        lorentz_x_sim[mask] * x[mask] + lorentz_y_sim[mask] * y[mask]
    ) / r[mask]

    # Analytical solution at this constant radius
    lorentz_radial_analytical = analytical_lorentz_radial(r)

    return {
        "z": z,
        "r": r,
        "lorentz_sim": lorentz_radial_sim,
        "lorentz_analytical": lorentz_radial_analytical,
        "lorentz_z_sim": lorentz_z_sim,
    }


# Process radial lines at different angles
print("=" * 70)
print("Lorentz Force Verification: MOOSE vs. Analytical")
print("=" * 70)

radial_lines = []
line_configs = [
    (f"{data_dir}/{geometry_name}_out_line_sample_0deg_0001.csv", "0° (x-axis)"),
    (f"{data_dir}/{geometry_name}_out_line_sample_45deg_0001.csv", "45°"),
    (f"{data_dir}/{geometry_name}_out_line_sample_90deg_0001.csv", "90° (y-axis)"),
]

for filename, name in line_configs:
    result = process_radial_line(filename, name)
    if result is not None:
        radial_lines.append(result)
    else:
        print(f"Warning: Skipping {name} due to missing file")

if not radial_lines:
    print("\nError: No radial line samples found. Please run the simulation first:")
    print(f"  <your-moose-app> -i {geometry_name}.i")
    exit(1)

# Print statistics for each radial line
print("\n1. RADIAL LINES (Axisymmetry Check)")
print("-" * 70)
for line in radial_lines:
    mask = line["mask"]
    max_abs_error = np.max(line["abs_error"][mask])
    mean_abs_error = np.mean(line["abs_error"][mask])
    max_rel_error = np.max(line["rel_error"][mask]) * 100
    mean_rel_error = np.mean(line["rel_error"][mask]) * 100

    # Statistics for z-component (expected to be zero)
    max_lorentz_z = np.max(np.abs(line["lorentz_z_sim"][mask]))
    mean_lorentz_z = np.mean(np.abs(line["lorentz_z_sim"][mask]))
    max_lorentz_radial = np.max(np.abs(line["lorentz_sim"][mask]))

    print(f"\n{line['name']}:")
    print(f"  Number of points:    {np.sum(mask)}")
    print(
        f"  Radial range:        {line['r'][mask].min():.4f} to {line['r'][mask].max():.4f} mm"
    )
    print(f"  Max absolute error:  {max_abs_error:.6e} N/mm³")
    print(f"  Mean absolute error: {mean_abs_error:.6e} N/mm³")
    print(f"  Max relative error:  {max_rel_error:.4f} %")
    print(f"  Mean relative error: {mean_rel_error:.4f} %")
    print(f"  Max |F_z|:           {max_lorentz_z:.6e} N/mm³ (should be ~0)")
    print(f"  Mean |F_z|:          {mean_lorentz_z:.6e} N/mm³ (should be ~0)")

    # Check if z-component is negligible compared to radial component
    z_to_radial_ratio = (
        max_lorentz_z / max_lorentz_radial if max_lorentz_radial > 0 else 0
    )
    if z_to_radial_ratio > 0.01:
        print(
            f"  ⚠ WARNING: F_z/F_r = {z_to_radial_ratio*100:.2f}% exceeds 1% threshold!"
        )
    else:
        print(f"  ✓ F_z negligible: F_z/F_r = {z_to_radial_ratio*100:.4f}%")

# Process axial line
print("\n2. AXIAL LINE (Uniformity Check)")
print("-" * 70)
axial_result = process_axial_line(
    f"{data_dir}/{geometry_name}_out_line_sample_axial_0001.csv"
)
if axial_result is not None:
    axial_abs_error = np.abs(
        axial_result["lorentz_sim"] - axial_result["lorentz_analytical"]
    )
    axial_variation = np.std(axial_result["lorentz_sim"])
    axial_mean = np.mean(axial_result["lorentz_sim"])

    # Statistics for z-component (expected to be zero)
    max_lorentz_z_axial = np.max(np.abs(axial_result["lorentz_z_sim"]))
    mean_lorentz_z_axial = np.mean(np.abs(axial_result["lorentz_z_sim"]))
    max_lorentz_radial_axial = np.max(np.abs(axial_result["lorentz_sim"]))

    print(f"\nAxial line at r = {axial_result['r'][0]:.4f} mm:")
    print(f"  Number of points:       {len(axial_result['z'])}")
    print(
        f"  Axial range:            {axial_result['z'].min():.4f} to {axial_result['z'].max():.4f} mm"
    )
    print(f"  Mean Lorentz force:     {axial_mean:.6e} N/mm³")
    print(f"  Std dev along axis:     {axial_variation:.6e} N/mm³")
    print(
        f"  Analytical value:       {axial_result['lorentz_analytical'][0]:.6e} N/mm³"
    )
    print(f"  Max absolute error:     {np.max(axial_abs_error):.6e} N/mm³")
    print(f"  Mean absolute error:    {np.mean(axial_abs_error):.6e} N/mm³")
    print(f"  Max |F_z|:              {max_lorentz_z_axial:.6e} N/mm³ (should be ~0)")
    print(f"  Mean |F_z|:             {mean_lorentz_z_axial:.6e} N/mm³ (should be ~0)")

    # Check if z-component is negligible compared to radial component
    z_to_radial_ratio_axial = (
        max_lorentz_z_axial / max_lorentz_radial_axial
        if max_lorentz_radial_axial > 0
        else 0
    )
    if z_to_radial_ratio_axial > 0.01:
        print(
            f"  ⚠ WARNING: F_z/F_r = {z_to_radial_ratio_axial*100:.2f}% exceeds 1% threshold!"
        )
    else:
        print(f"  ✓ F_z negligible: F_z/F_r = {z_to_radial_ratio_axial*100:.4f}%")

    # Check if axial variation is small (indicates uniformity)
    relative_variation = (
        axial_variation / np.abs(axial_mean) * 100 if axial_mean != 0 else 0
    )
    print(f"  Relative variation:     {relative_variation:.4f} %")
    if relative_variation < 1.0:
        print("  ✓ Axial uniformity confirmed (variation < 1%)")
    else:
        print("  ⚠ Significant axial variation detected")
else:
    print("\nWarning: Axial line sample not found")

print("\n" + "=" * 70)

# Create comprehensive comparison plots
fig = plt.figure(figsize=(14, 10))
gs = fig.add_gridspec(3, 2, hspace=0.3, wspace=0.3)

# Plot 1: Radial Lorentz force comparison for all angles
ax1 = fig.add_subplot(gs[0, 0])
colors = ["red", "green", "blue"]
for i, line in enumerate(radial_lines):
    mask = line["mask"]
    ax1.plot(
        line["r"][mask],
        line["lorentz_sim"][mask],
        "o",
        color=colors[i],
        markersize=3,
        alpha=0.6,
        label=f"MOOSE {line['name']}",
    )
if radial_lines:
    mask = radial_lines[0]["mask"]
    ax1.plot(
        radial_lines[0]["r"][mask],
        radial_lines[0]["lorentz_analytical"][mask],
        "k-",
        linewidth=2,
        label="Analytical",
    )
ax1.set_xlabel("Radius (mm)", fontsize=11)
ax1.set_ylabel("Radial Lorentz Force (N/mm³)", fontsize=11)
ax1.set_title("Radial Force: Axisymmetry Check", fontsize=12, fontweight="bold")
ax1.legend(fontsize=9, loc="best")
ax1.grid(True, alpha=0.3)

# Plot 2: Absolute error for all radial lines
ax2 = fig.add_subplot(gs[0, 1])
for i, line in enumerate(radial_lines):
    mask = line["mask"]
    ax2.plot(
        line["r"][mask],
        line["abs_error"][mask],
        color=colors[i],
        linewidth=1.5,
        label=line["name"],
    )
ax2.set_xlabel("Radius (mm)", fontsize=11)
ax2.set_ylabel("Absolute Error (N/mm³)", fontsize=11)
ax2.set_title("Absolute Error by Angle", fontsize=12, fontweight="bold")
ax2.legend(fontsize=9)
ax2.grid(True, alpha=0.3)
ax2.axhline(y=0, color="k", linestyle="--", linewidth=1)

# Plot 3: Relative error for all radial lines
ax3 = fig.add_subplot(gs[1, 0])
for i, line in enumerate(radial_lines):
    mask = line["mask"]
    ax3.plot(
        line["r"][mask],
        line["rel_error"][mask] * 100,
        color=colors[i],
        linewidth=1.5,
        label=line["name"],
    )
ax3.set_xlabel("Radius (mm)", fontsize=11)
ax3.set_ylabel("Relative Error (%)", fontsize=11)
ax3.set_title("Relative Error by Angle", fontsize=12, fontweight="bold")
ax3.legend(fontsize=9)
ax3.grid(True, alpha=0.3)
ax3.axhline(y=0, color="k", linestyle="--", linewidth=1)

# Plot 4: Axial uniformity check
ax4 = fig.add_subplot(gs[1, 1])
if axial_result is not None:
    ax4.plot(
        axial_result["z"], axial_result["lorentz_sim"], "b-", linewidth=2, label="MOOSE"
    )
    ax4.axhline(
        y=axial_result["lorentz_analytical"][0],
        color="k",
        linestyle="--",
        linewidth=2,
        label="Analytical",
    )
    ax4.set_xlabel("Axial Position z (mm)", fontsize=11)
    ax4.set_ylabel("Radial Lorentz Force (N/mm³)", fontsize=11)
    ax4.set_title(
        f"Axial Uniformity at r = {axial_result['r'][0]:.2f} mm",
        fontsize=12,
        fontweight="bold",
    )
    ax4.legend(fontsize=9)
    ax4.grid(True, alpha=0.3)

# Plot 5: Combined error statistics
ax5 = fig.add_subplot(gs[2, :])
if radial_lines:
    angles = [line["name"] for line in radial_lines]
    max_errors = [np.max(line["abs_error"][line["mask"]]) for line in radial_lines]
    mean_errors = [np.mean(line["abs_error"][line["mask"]]) for line in radial_lines]

    x = np.arange(len(angles))
    width = 0.35

    ax5.bar(x - width / 2, max_errors, width, label="Max Error", color="indianred")
    ax5.bar(x + width / 2, mean_errors, width, label="Mean Error", color="steelblue")
    ax5.set_xlabel("Radial Line", fontsize=11)
    ax5.set_ylabel("Absolute Error (N/mm³)", fontsize=11)
    ax5.set_title("Error Summary Across All Lines", fontsize=12, fontweight="bold")
    ax5.set_xticks(x)
    ax5.set_xticklabels(angles)
    ax5.legend(fontsize=9)
    ax5.grid(True, alpha=0.3, axis="y")

plt.savefig(f"{figures_dir}/lorentz_verification.png", dpi=150, bbox_inches="tight")
print(f"\nPlot saved as '{figures_dir}/lorentz_verification.png'")
plt.close(fig)

# Create separate plot for 0 degree line error metrics
if radial_lines:
    # Find the 0 degree line
    line_0deg = None
    for line in radial_lines:
        if "0°" in line["name"]:
            line_0deg = line
            break

    if line_0deg is not None:
        fig2 = plt.figure(figsize=(15, 5))

        mask = line_0deg["mask"]

        # Plot 1: Comparison of simulated vs analytical
        ax1 = fig2.add_subplot(1, 3, 1)
        ax1.plot(
            line_0deg["r"][mask],
            line_0deg["lorentz_sim"][mask],
            "o",
            color="red",
            markersize=4,
            alpha=0.6,
            label="MOOSE 0°",
        )
        ax1.plot(
            line_0deg["r"][mask],
            line_0deg["lorentz_analytical"][mask],
            "k-",
            linewidth=2,
            label="Analytical",
        )
        ax1.set_xlabel("Radius (mm)", fontsize=12)
        ax1.set_ylabel("Radial Lorentz Force (N/mm³)", fontsize=12)
        ax1.set_title("0° Line: MOOSE vs Analytical", fontsize=13, fontweight="bold")
        ax1.legend(fontsize=10)
        ax1.grid(True, alpha=0.3)

        # Plot 2: Absolute error for 0 degree line
        ax2 = fig2.add_subplot(1, 3, 2)
        ax2.plot(
            line_0deg["r"][mask],
            line_0deg["abs_error"][mask],
            "o-",
            color="red",
            linewidth=2,
            markersize=4,
            label="0° line",
        )
        ax2.set_xlabel("Radius (mm)", fontsize=12)
        ax2.set_ylabel("Absolute Error (N/mm³)", fontsize=12)
        ax2.set_title("0° Line: Absolute Error", fontsize=13, fontweight="bold")
        ax2.legend(fontsize=10)
        ax2.grid(True, alpha=0.3)
        ax2.axhline(y=0, color="k", linestyle="--", linewidth=1)

        # Plot 3: Relative error for 0 degree line
        ax3 = fig2.add_subplot(1, 3, 3)
        ax3.plot(
            line_0deg["r"][mask],
            line_0deg["rel_error"][mask] * 100,
            "o-",
            color="red",
            linewidth=2,
            markersize=4,
            label="0° line",
        )
        ax3.set_xlabel("Radius (mm)", fontsize=12)
        ax3.set_ylabel("Relative Error (%)", fontsize=12)
        ax3.set_title("0° Line: Relative Error", fontsize=13, fontweight="bold")
        ax3.legend(fontsize=10)
        ax3.grid(True, alpha=0.3)
        ax3.axhline(y=0, color="k", linestyle="--", linewidth=1)

        plt.tight_layout()
        plt.savefig(
            f"{figures_dir}/lorentz_0deg_error.png", dpi=150, bbox_inches="tight"
        )
        print(f"Plot saved as '{figures_dir}/lorentz_0deg_error.png'")
        plt.close(fig2)
    else:
        print("\nWarning: 0° line data not found for error plot")

# Close log file
log_file.close()
