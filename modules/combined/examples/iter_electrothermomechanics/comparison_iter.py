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
ITER Cable Stress Comparison Script

Compare stress quantities (von Mises, stress_zz, stress_xx, stress_xy) across
different ITER cable geometries over time.

Usage:
    python comparison_iter.py [--data-dir DIR] [--output-dir DIR]

Geometries:
    - copper_cylinder: Solid copper cylinder
    - cylinder: Cu-Nb3Sn effective cylinder
    - annulus: Cu-Nb3Sn annulus (bundle only)
    - cable: Full cable with jacket

Generates plots for:
    - Global average stress quantities (all geometries)
    - Bundle comparison (all geometries - uses global for single-block, bundle for cable)
    - Jacket-only stress evolution (cable only, saved to figures/cable/)
"""

import argparse
import shutil
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd

# Plotting style
plt.style.use("seaborn-v0_8-darkgrid")

# Geometry configurations
GEOMETRIES = {
    "copper_cylinder": {
        "name": "Copper Cylinder",
        "color": "#1f77b4",
        "marker": "o",
        "blocks": [],  # Single block - global postprocessors ARE bundle postprocessors
    },
    "cylinder": {
        "name": "Cu-Nb3Sn Cylinder",
        "color": "#ff7f0e",
        "marker": "s",
        "blocks": [],  # Single block - global postprocessors ARE bundle postprocessors
    },
    "annulus": {
        "name": "Cu-Nb3Sn Annulus",
        "color": "#2ca02c",
        "marker": "^",
        "blocks": [],  # Single block (channel deleted) - global postprocessors ARE bundle postprocessors
    },
    "cable": {
        "name": "Full Cable",
        "color": "#d62728",
        "marker": "D",
        "blocks": [
            "bundle",
            "jacket",
        ],  # Multi-block with block-specific postprocessors
    },
}

# Stress quantities to plot
STRESS_QUANTITIES = {
    "vonmises_stress_pp": {
        "title": "von Mises Stress",
        "ylabel": r"von Mises Stress (MPa)",
        "filename": "vonmises_stress_comparison.png",
    },
    "stress_zz_pp": {
        "title": r"Axial Stress $\sigma_{zz}$",
        "ylabel": r"Stress $\sigma_{zz}$ (MPa)",
        "filename": "stress_zz_comparison.png",
    },
    "stress_xx_pp": {
        "title": r"Radial Stress $\sigma_{xx}$",
        "ylabel": r"Stress $\sigma_{xx}$ (MPa)",
        "filename": "stress_xx_comparison.png",
    },
    "stress_xy_pp": {
        "title": r"Shear Stress $\sigma_{xy}$",
        "ylabel": r"Stress $\sigma_{xy}$ (MPa)",
        "filename": "stress_xy_comparison.png",
    },
}


def load_geometry_data(geometry_name, data_dir):
    """
    Load CSV data for a given geometry.

    Args:
        geometry_name: Name of geometry (e.g., 'copper_cylinder')
        data_dir: Base data directory path

    Returns:
        pandas.DataFrame with postprocessor data, or None if file not found

    """
    csv_path = data_dir / geometry_name / f"{geometry_name}_out.csv"

    if not csv_path.exists():
        print(f"Warning: CSV file not found for {geometry_name}: {csv_path}")
        return None

    try:
        df = pd.read_csv(csv_path)
        return df
    except Exception as e:
        print(f"Error loading {csv_path}: {e}")
        return None


def plot_global_comparison(data_dict, quantity_key, quantity_info, output_dir):
    """
    Plot global average stress comparison across all geometries.

    Args:
        data_dict: Dictionary of {geometry_name: DataFrame}
        quantity_key: Postprocessor name (e.g., 'stress_zz_pp')
        quantity_info: Dictionary with plot formatting info
        output_dir: Directory to save figure

    """
    fig, ax = plt.subplots(figsize=(10, 6))

    for geom_name, config in GEOMETRIES.items():
        if geom_name not in data_dict or data_dict[geom_name] is None:
            continue

        df = data_dict[geom_name]

        if quantity_key not in df.columns:
            print(f"Warning: {quantity_key} not found in {geom_name} data")
            continue

        # Plot time vs stress
        ax.plot(
            df["time"],
            df[quantity_key],
            label=config["name"],
            color=config["color"],
            marker=config["marker"],
            markevery=max(1, len(df) // 10),
            linewidth=2,
            markersize=6,
        )

    ax.set_xlabel("Time (s)", fontsize=12)
    ax.set_ylabel(quantity_info["ylabel"], fontsize=12)
    ax.set_title(
        f"{quantity_info['title']} - Global Average", fontsize=14, fontweight="bold"
    )
    ax.legend(loc="best", fontsize=10)
    ax.grid(True, alpha=0.3)

    plt.tight_layout()

    output_path = output_dir / quantity_info["filename"]
    plt.savefig(output_path, dpi=300, bbox_inches="tight")
    print(f"Saved: {output_path}")
    plt.close()


def plot_bundle_comparison(data_dict, quantity_key, quantity_info, output_dir):
    """
    Plot bundle stress comparison across all geometries.

    For single-block geometries (copper_cylinder, cylinder, annulus), uses global average.
    For multi-block cable, uses bundle-specific postprocessor.

    Args:
        data_dict: Dictionary of {geometry_name: DataFrame}
        quantity_key: Base postprocessor name (e.g., 'stress_zz_pp')
        quantity_info: Dictionary with plot formatting info
        output_dir: Directory to save figure

    """
    # Extract base quantity name (remove _pp suffix)
    base_quantity = quantity_key.replace("_pp", "")

    fig, ax = plt.subplots(figsize=(10, 6))

    for geom_name, config in GEOMETRIES.items():
        if geom_name not in data_dict or data_dict[geom_name] is None:
            continue

        df = data_dict[geom_name]

        # For cable, use bundle-specific postprocessor; for others use global
        if geom_name == "cable":
            column = f"{base_quantity}_bundle"
            if column not in df.columns:
                continue
        else:
            column = quantity_key
            if column not in df.columns:
                continue

        label = f"{config['name']}"

        ax.plot(
            df["time"],
            df[column],
            label=label,
            color=config["color"],
            marker=config["marker"],
            markevery=max(1, len(df) // 10),
            linewidth=2,
            markersize=6,
        )

    ax.set_xlabel("Time (s)", fontsize=12)
    ax.set_ylabel(quantity_info["ylabel"], fontsize=12)
    ax.set_title(
        f"{quantity_info['title']} - Bundle Comparison", fontsize=14, fontweight="bold"
    )
    ax.legend(loc="best", fontsize=10)
    ax.grid(True, alpha=0.3)

    plt.tight_layout()

    filename = quantity_info["filename"].replace(".png", "_bundle.png")
    output_path = output_dir / filename
    plt.savefig(output_path, dpi=300, bbox_inches="tight")
    print(f"Saved: {output_path}")
    plt.close()


def plot_jacket_only(data_dict, output_dir):
    """
    Plot jacket-only stress evolution for cable geometry.
    Saved to figures/cable/ directory.

    Args:
        data_dict: Dictionary of {geometry_name: DataFrame}
        output_dir: Base output directory (figures/)

    """
    if "cable" not in data_dict or data_dict["cable"] is None:
        print("Warning: Cable data not available for jacket-only plot")
        return

    df = data_dict["cable"]
    config = GEOMETRIES["cable"]

    # Create cable-specific output directory
    cable_dir = output_dir / "cable"
    cable_dir.mkdir(parents=True, exist_ok=True)

    # Plot all stress components for jacket
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    fig.suptitle("Jacket Stress Evolution - Full Cable", fontsize=16, fontweight="bold")

    stress_list = [
        ("vonmises_stress_jacket", "von Mises Stress", r"von Mises Stress (MPa)"),
        (
            "stress_zz_jacket",
            r"Axial Stress $\sigma_{zz}$",
            r"Stress $\sigma_{zz}$ (MPa)",
        ),
        (
            "stress_xx_jacket",
            r"Radial Stress $\sigma_{xx}$",
            r"Stress $\sigma_{xx}$ (MPa)",
        ),
        (
            "stress_xy_jacket",
            r"Shear Stress $\sigma_{xy}$",
            r"Stress $\sigma_{xy}$ (MPa)",
        ),
    ]

    for idx, (stress_key, title, ylabel) in enumerate(stress_list):
        ax = axes[idx // 2, idx % 2]

        if stress_key not in df.columns:
            ax.text(
                0.5,
                0.5,
                f"{stress_key}\nNot Available",
                ha="center",
                va="center",
                transform=ax.transAxes,
            )
            continue

        ax.plot(
            df["time"],
            df[stress_key],
            label="Jacket",
            color=config["color"],
            linewidth=2.5,
            marker=config["marker"],
            markersize=6,
            markevery=max(1, len(df) // 10),
        )

        ax.set_xlabel("Time (s)", fontsize=12)
        ax.set_ylabel(ylabel, fontsize=12)
        ax.set_title(title, fontsize=13)
        ax.legend(loc="best", fontsize=11)
        ax.grid(True, alpha=0.3)

    plt.tight_layout()

    output_path = cable_dir / "jacket_stress_evolution.png"
    plt.savefig(output_path, dpi=300, bbox_inches="tight")
    print(f"Saved: {output_path}")
    plt.close()


def plot_temperature_comparison(data_dict, output_dir):
    """
    Plot temperature ramp comparison (should be identical for all geometries).

    Args:
        data_dict: Dictionary of {geometry_name: DataFrame}
        output_dir: Directory to save figure

    """
    fig, ax = plt.subplots(figsize=(10, 6))

    for geom_name, config in GEOMETRIES.items():
        if geom_name not in data_dict or data_dict[geom_name] is None:
            continue

        df = data_dict[geom_name]

        if "temperature_average" not in df.columns:
            continue

        ax.plot(
            df["time"],
            df["temperature_average"],
            label=config["name"],
            color=config["color"],
            marker=config["marker"],
            markevery=max(1, len(df) // 10),
            linewidth=2,
            markersize=6,
        )

    ax.set_xlabel("Time (s)", fontsize=12)
    ax.set_ylabel("Temperature (K)", fontsize=12)
    ax.set_title("Temperature Ramp - All Geometries", fontsize=14, fontweight="bold")
    ax.legend(loc="best", fontsize=10)
    ax.grid(True, alpha=0.3)

    plt.tight_layout()

    output_path = output_dir / "temperature_comparison.png"
    plt.savefig(output_path, dpi=300, bbox_inches="tight")
    print(f"Saved: {output_path}")
    plt.close()


def plot_all_stresses_combined(data_dict, output_dir):
    """
    Plot all stress components in a single multi-panel figure for each geometry.

    Args:
        data_dict: Dictionary of {geometry_name: DataFrame}
        output_dir: Directory to save figure

    """
    for geom_name, config in GEOMETRIES.items():
        if geom_name not in data_dict or data_dict[geom_name] is None:
            continue

        df = data_dict[geom_name]

        fig, axes = plt.subplots(2, 2, figsize=(14, 10))
        fig.suptitle(
            f"Stress Evolution - {config['name']}", fontsize=16, fontweight="bold"
        )

        stress_list = [
            ("vonmises_stress_pp", "von Mises Stress", r"von Mises Stress (MPa)"),
            ("stress_zz_pp", r"Stress $\sigma_{zz}$", r"Stress $\sigma_{zz}$ (MPa)"),
            ("stress_xx_pp", r"Stress $\sigma_{xx}$", r"Stress $\sigma_{xx}$ (MPa)"),
            ("stress_xy_pp", r"Stress $\sigma_{xy}$", r"Stress $\sigma_{xy}$ (MPa)"),
        ]

        for idx, (stress_key, title, ylabel) in enumerate(stress_list):
            ax = axes[idx // 2, idx % 2]

            if stress_key not in df.columns:
                ax.text(
                    0.5,
                    0.5,
                    f"{stress_key}\nNot Available",
                    ha="center",
                    va="center",
                    transform=ax.transAxes,
                )
                continue

            # Global average
            ax.plot(
                df["time"],
                df[stress_key],
                label="Global Average",
                color=config["color"],
                linewidth=2,
                marker="o",
                markevery=max(1, len(df) // 10),
            )

            # Block-specific if available
            base_quantity = stress_key.replace("_pp", "")
            for block in config["blocks"]:
                block_column = f"{base_quantity}_{block}"
                if block_column in df.columns:
                    linestyle = "--" if block == "jacket" else "-."
                    ax.plot(
                        df["time"],
                        df[block_column],
                        label=f"{block.capitalize()}",
                        linestyle=linestyle,
                        linewidth=2,
                        marker="s",
                        markevery=max(1, len(df) // 10),
                    )

            ax.set_xlabel("Time (s)", fontsize=10)
            ax.set_ylabel(ylabel, fontsize=10)
            ax.set_title(title, fontsize=11)
            ax.legend(loc="best", fontsize=9)
            ax.grid(True, alpha=0.3)

        plt.tight_layout()

        output_path = output_dir / f"{geom_name}_stress_evolution.png"
        plt.savefig(output_path, dpi=300, bbox_inches="tight")
        print(f"Saved: {output_path}")
        plt.close()


def plot_individual_stress(data_dict, figures_dir):
    """
    Plot individual stress components for each geometry in their figures directory.

    Args:
        data_dict: Dictionary of {geometry_name: DataFrame}
        figures_dir: Base figures directory path

    """
    stress_list = [
        ("vonmises_stress_pp", "von Mises Stress", r"von Mises Stress (MPa)"),
        ("stress_zz_pp", r"Axial Stress $\sigma_{zz}$", r"Stress $\sigma_{zz}$ (MPa)"),
        ("stress_xx_pp", r"Radial Stress $\sigma_{xx}$", r"Stress $\sigma_{xx}$ (MPa)"),
        ("stress_xy_pp", r"Shear Stress $\sigma_{xy}$", r"Stress $\sigma_{xy}$ (MPa)"),
    ]

    for geom_name, config in GEOMETRIES.items():
        if geom_name not in data_dict or data_dict[geom_name] is None:
            continue

        df = data_dict[geom_name]

        # Create figures subdirectory for this geometry
        geom_figures_dir = figures_dir / geom_name
        geom_figures_dir.mkdir(parents=True, exist_ok=True)

        for stress_key, title, ylabel in stress_list:
            if stress_key not in df.columns:
                continue

            fig, ax = plt.subplots(figsize=(10, 6))

            # Global average
            ax.plot(
                df["time"],
                df[stress_key],
                label="Global Average",
                color=config["color"],
                linewidth=2.5,
                marker="o",
                markersize=6,
                markevery=max(1, len(df) // 10),
            )

            # Block-specific if available
            base_quantity = stress_key.replace("_pp", "")
            for block in config["blocks"]:
                block_column = f"{base_quantity}_{block}"
                if block_column in df.columns:
                    linestyle = "--" if block == "jacket" else "-."
                    ax.plot(
                        df["time"],
                        df[block_column],
                        label=f"{block.capitalize()}",
                        linestyle=linestyle,
                        linewidth=2.5,
                        marker="s",
                        markersize=6,
                        markevery=max(1, len(df) // 10),
                    )

            ax.set_xlabel("Time (s)", fontsize=12)
            ax.set_ylabel(ylabel, fontsize=12)
            ax.set_title(f"{title} - {config['name']}", fontsize=14, fontweight="bold")
            ax.legend(loc="best", fontsize=11)
            ax.grid(True, alpha=0.3)

            plt.tight_layout()

            # Save to geometry's data directory
            filename = f"{geom_name}_{stress_key}.png"
            output_path = geom_figures_dir / filename
            plt.savefig(output_path, dpi=300, bbox_inches="tight")
            print(f"Saved: {output_path}")
            plt.close()


def main():
    parser = argparse.ArgumentParser(
        description="Compare stress quantities across ITER cable geometries",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument(
        "--data-dir",
        type=str,
        default="data",
        help="Base data directory (default: data)",
    )
    parser.add_argument(
        "--output-dir",
        type=str,
        default="figures/comparison",
        help="Output directory for plots (default: figures/comparison)",
    )

    args = parser.parse_args()

    # Setup paths
    data_dir = Path(args.data_dir)
    output_dir = Path(args.output_dir)

    if not data_dir.exists():
        print(f"Error: Data directory does not exist: {data_dir}")
        sys.exit(1)

    # Wipe and recreate output directory to remove stale plots
    if output_dir.exists():
        print(f"Removing existing comparison directory: {output_dir}")
        shutil.rmtree(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    # Load all geometry data
    print("Loading geometry data...")
    data_dict = {}
    for geom_name in GEOMETRIES:
        data_dict[geom_name] = load_geometry_data(geom_name, data_dir)

    # Check if any data was loaded
    if all(df is None for df in data_dict.values()):
        print("Error: No data files found. Run simulations first.")
        sys.exit(1)

    # Generate plots
    print("\nGenerating comparison plots...")

    # Temperature comparison
    plot_temperature_comparison(data_dict, output_dir)

    # Global and bundle stress comparisons
    for quantity_key, quantity_info in STRESS_QUANTITIES.items():
        plot_global_comparison(data_dict, quantity_key, quantity_info, output_dir)
        plot_bundle_comparison(data_dict, quantity_key, quantity_info, output_dir)

    # Jacket-only plot (saved to figures/cable/)
    plot_jacket_only(data_dict, output_dir.parent)

    # Combined stress evolution for each geometry (saved to comparison dir)
    plot_all_stresses_combined(data_dict, output_dir)

    # Individual stress plots for each geometry (saved to their figures directories)
    print("\nGenerating individual geometry plots...")
    plot_individual_stress(data_dict, output_dir.parent)

    print(f"\nComparison plots saved to: {output_dir}")
    print("Individual plots saved to: figures/<geometry>/")
    print("Done!")


if __name__ == "__main__":
    main()
