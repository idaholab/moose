#!/usr/bin/env python3
"""Plot original Martin-Moyce rectangular paper data from a local workbook.

This script reads the digitized workbook directly from the .xlsx archive using
only the Python standard library, then writes a two-panel SVG:
  - front position in paper variables (T, Z)
  - top-of-column height in paper variables (tau, H)
"""

from __future__ import annotations

import argparse
import csv
import html
import math
import zipfile
from dataclasses import dataclass
from pathlib import Path
from typing import Dict
from xml.etree import ElementTree as ET


MAIN_NS = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
REL_NS = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
PKG_REL_NS = "http://schemas.openxmlformats.org/package/2006/relationships"


@dataclass(frozen=True)
class Point:
    x: float
    y: float


@dataclass(frozen=True)
class Series:
    label: str
    points: list[Point]
    color: str
    show_points: bool = True
    stroke_width: float = 2.4


def xml_ns(tag: str, namespace: str = MAIN_NS) -> str:
    return f"{{{namespace}}}{tag}"


def excel_col_to_index(column: str) -> int:
    index = 0
    for char in column:
        index = index * 26 + ord(char.upper()) - ord("A") + 1
    return index - 1


def parse_excel_ref(cell_ref: str) -> tuple[int, int]:
    column = "".join(ch for ch in cell_ref if ch.isalpha())
    row = "".join(ch for ch in cell_ref if ch.isdigit())
    return int(row) - 1, excel_col_to_index(column)


def load_shared_strings(archive: zipfile.ZipFile) -> list[str]:
    if "xl/sharedStrings.xml" not in archive.namelist():
        return []

    root = ET.fromstring(archive.read("xl/sharedStrings.xml"))
    shared_strings = []
    for item in root.findall(xml_ns("si")):
        text = "".join(node.text or "" for node in item.iterfind(f".//{xml_ns('t')}"))
        shared_strings.append(text)
    return shared_strings


def workbook_sheet_paths(archive: zipfile.ZipFile) -> dict[str, str]:
    workbook_root = ET.fromstring(archive.read("xl/workbook.xml"))
    rels_root = ET.fromstring(archive.read("xl/_rels/workbook.xml.rels"))

    rel_id_to_target: dict[str, str] = {}
    for rel in rels_root.findall(xml_ns("Relationship", PKG_REL_NS)):
        rel_id_to_target[rel.attrib["Id"]] = rel.attrib["Target"]

    sheet_paths = {}
    sheets = workbook_root.find(xml_ns("sheets"))
    if sheets is None:
        return sheet_paths

    for sheet in sheets.findall(xml_ns("sheet")):
        rel_id = sheet.attrib[f"{{{REL_NS}}}id"]
        target = rel_id_to_target[rel_id]
        if target.startswith("/"):
            target = target.lstrip("/")
        elif not target.startswith("xl/"):
            target = f"xl/{target}"
        sheet_paths[sheet.attrib["name"]] = target
    return sheet_paths


def read_sheet_rows(archive: zipfile.ZipFile, sheet_path: str, shared_strings: list[str]) -> list[list[str]]:
    root = ET.fromstring(archive.read(sheet_path))
    rows_out: list[list[str]] = []
    sheet_data = root.find(xml_ns("sheetData"))
    if sheet_data is None:
        return rows_out

    for row in sheet_data.findall(xml_ns("row")):
        values: Dict[int, str] = {}
        max_col = -1
        for cell in row.findall(xml_ns("c")):
            cell_ref = cell.attrib.get("r", "")
            _, col_index = parse_excel_ref(cell_ref)
            max_col = max(max_col, col_index)
            cell_type = cell.attrib.get("t", "")
            value_node = cell.find(xml_ns("v"))
            if value_node is None:
                values[col_index] = ""
                continue
            raw = value_node.text or ""
            if cell_type == "s":
                values[col_index] = shared_strings[int(raw)]
            else:
                values[col_index] = raw

        if max_col >= 0:
            rows_out.append([values.get(i, "") for i in range(max_col + 1)])

    return rows_out


def rows_to_dicts(rows: list[list[str]]) -> list[dict[str, str]]:
    if not rows:
        return []
    headers = rows[0]
    return [dict(zip(headers, row + [""] * (len(headers) - len(row)))) for row in rows[1:]]


def load_workbook_tables(path: Path) -> tuple[list[dict[str, str]], list[dict[str, str]]]:
    with zipfile.ZipFile(path) as archive:
        shared_strings = load_shared_strings(archive)
        sheet_paths = workbook_sheet_paths(archive)
        front_rows = read_sheet_rows(archive, sheet_paths["Front_mean_rectangular"], shared_strings)
        height_rows = read_sheet_rows(
            archive, sheet_paths["Top_height_mean_rectangular"], shared_strings
        )
    return rows_to_dicts(front_rows), rows_to_dicts(height_rows)


def filter_rows(
    rows: list[dict[str, str]], n_squared: int, a_value: str, x_key: str, y_key: str
) -> dict[str, list[Point]]:
    grouped: dict[str, list[Point]] = {}
    for row in rows:
        if int(float(row["n_squared"])) != n_squared:
            continue
        if a_value != "all" and not math.isclose(float(row["a_in"]), float(a_value), rel_tol=0.0, abs_tol=1e-12):
            continue
        a_key = row["a_in"]
        grouped.setdefault(a_key, []).append(Point(float(row[x_key]), float(row[y_key])))

    for points in grouped.values():
        points.sort(key=lambda point: point.x)
    return grouped


def nice_ticks(data_min: float, data_max: float, tick_count: int = 5) -> list[float]:
    if math.isclose(data_min, data_max):
        return [data_min]

    span = data_max - data_min
    raw_step = span / max(tick_count - 1, 1)
    magnitude = 10 ** math.floor(math.log10(raw_step))
    for multiplier in (1, 2, 5, 10):
        step = multiplier * magnitude
        if raw_step <= step:
            break

    start = math.floor(data_min / step) * step
    stop = math.ceil(data_max / step) * step

    ticks = []
    value = start
    while value <= stop + step * 0.5:
        ticks.append(round(value, 10))
        value += step
    return ticks


def format_tick(value: float) -> str:
    if math.isclose(value, round(value), abs_tol=1e-10):
        return str(int(round(value)))
    return f"{value:.2f}".rstrip("0").rstrip(".")


def svg_line(x1: float, y1: float, x2: float, y2: float, **attrs: str) -> str:
    pieces = [f'<line x1="{x1:.2f}" y1="{y1:.2f}" x2="{x2:.2f}" y2="{y2:.2f}"']
    pieces.extend(f' {key}="{html.escape(value)}"' for key, value in attrs.items())
    pieces.append(" />")
    return "".join(pieces)


def svg_text(x: float, y: float, text: str, **attrs: str) -> str:
    pieces = [f'<text x="{x:.2f}" y="{y:.2f}"']
    pieces.extend(f' {key}="{html.escape(value)}"' for key, value in attrs.items())
    pieces.append(f">{html.escape(text)}</text>")
    return "".join(pieces)


def build_panel(
    x0: float,
    y0: float,
    width: float,
    height: float,
    title: str,
    x_label: str,
    y_label: str,
    series_list: list[Series],
) -> list[str]:
    margin_left = 80.0
    margin_right = 20.0
    margin_top = 45.0
    margin_bottom = 65.0

    plot_x0 = x0 + margin_left
    plot_y0 = y0 + margin_top
    plot_w = width - margin_left - margin_right
    plot_h = height - margin_top - margin_bottom

    x_values = [point.x for series in series_list for point in series.points]
    y_values = [point.y for series in series_list for point in series.points]
    x_min = min(x_values)
    x_max = max(x_values)
    y_min = min(y_values)
    y_max = max(y_values)

    x_pad = 0.05 * (x_max - x_min or 1.0)
    y_pad = 0.08 * (y_max - y_min or 1.0)
    x_min -= x_pad
    x_max += x_pad
    y_min -= y_pad
    y_max += y_pad

    def x_map(value: float) -> float:
        return plot_x0 + (value - x_min) * plot_w / (x_max - x_min)

    def y_map(value: float) -> float:
        return plot_y0 + plot_h - (value - y_min) * plot_h / (y_max - y_min)

    parts = [
        f'<rect x="{x0:.2f}" y="{y0:.2f}" width="{width:.2f}" height="{height:.2f}" fill="white" />',
        svg_text(
            x0 + width / 2.0,
            y0 + 25.0,
            title,
            **{"text-anchor": "middle", "font-size": "18", "font-weight": "600", "font-family": "Helvetica, Arial, sans-serif"},
        ),
    ]

    x_ticks = nice_ticks(x_min, x_max, 6)
    y_ticks = nice_ticks(y_min, y_max, 6)

    for tick in x_ticks:
        x = x_map(tick)
        parts.append(
            svg_line(
                x,
                plot_y0,
                x,
                plot_y0 + plot_h,
                stroke="#e3e7ee",
                **{"stroke-width": "1"},
            )
        )
        parts.append(
            svg_text(
                x,
                plot_y0 + plot_h + 22.0,
                format_tick(tick),
                **{"text-anchor": "middle", "font-size": "12", "fill": "#374151", "font-family": "Helvetica, Arial, sans-serif"},
            )
        )

    for tick in y_ticks:
        y = y_map(tick)
        parts.append(
            svg_line(
                plot_x0,
                y,
                plot_x0 + plot_w,
                y,
                stroke="#e3e7ee",
                **{"stroke-width": "1"},
            )
        )
        parts.append(
            svg_text(
                plot_x0 - 12.0,
                y + 4.0,
                format_tick(tick),
                **{"text-anchor": "end", "font-size": "12", "fill": "#374151", "font-family": "Helvetica, Arial, sans-serif"},
            )
        )

    parts.append(
        svg_line(
            plot_x0,
            plot_y0 + plot_h,
            plot_x0 + plot_w,
            plot_y0 + plot_h,
            stroke="#111827",
            **{"stroke-width": "1.5"},
        )
    )
    parts.append(
        svg_line(
            plot_x0,
            plot_y0,
            plot_x0,
            plot_y0 + plot_h,
            stroke="#111827",
            **{"stroke-width": "1.5"},
        )
    )

    for series in series_list:
        polyline_points = " ".join(
            f"{x_map(point.x):.2f},{y_map(point.y):.2f}" for point in series.points
        )
        parts.append(
            f'<polyline fill="none" stroke="{series.color}" stroke-width="{series.stroke_width:.1f}" '
            f'stroke-linejoin="round" stroke-linecap="round" points="{polyline_points}" />'
        )
        if series.show_points:
            for point in series.points:
                parts.append(
                    f'<circle cx="{x_map(point.x):.2f}" cy="{y_map(point.y):.2f}" r="3.8" '
                    f'fill="{series.color}" stroke="white" stroke-width="1.2" />'
                )

    legend_x = plot_x0 + 10.0
    legend_y = plot_y0 + 14.0
    for index, series in enumerate(series_list):
        entry_y = legend_y + index * 20.0
        parts.append(
            svg_line(
                legend_x,
                entry_y,
                legend_x + 18.0,
                entry_y,
                stroke=series.color,
                **{"stroke-width": "3"},
            )
        )
        parts.append(
            svg_text(
                legend_x + 24.0,
                entry_y + 4.0,
                series.label,
                **{"font-size": "12", "fill": "#111827", "font-family": "Helvetica, Arial, sans-serif"},
            )
        )

    parts.append(
        svg_text(
            plot_x0 + plot_w / 2.0,
            y0 + height - 20.0,
            x_label,
            **{"text-anchor": "middle", "font-size": "14", "fill": "#111827", "font-family": "Helvetica, Arial, sans-serif"},
        )
    )
    parts.append(
        f'<g transform="translate({x0 + 22.0:.2f},{plot_y0 + plot_h / 2.0:.2f}) rotate(-90)">'
        f'{svg_text(0.0, 0.0, y_label, **{"text-anchor": "middle", "font-size": "14", "fill": "#111827", "font-family": "Helvetica, Arial, sans-serif"})}'
        "</g>"
    )

    return parts


def make_svg(front_series: list[Series], height_series: list[Series], title: str) -> str:
    width = 1280
    height = 560
    panel_width = 600
    panel_height = 480
    gutter = 40
    left_x = 20
    top_y = 55
    right_x = left_x + panel_width + gutter

    parts = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
        '<rect width="100%" height="100%" fill="#f8fafc" />',
        svg_text(
            width / 2.0,
            30.0,
            title,
            **{"text-anchor": "middle", "font-size": "24", "font-weight": "700", "fill": "#111827", "font-family": "Helvetica, Arial, sans-serif"},
        ),
    ]
    parts.extend(
        build_panel(
            left_x,
            top_y,
            panel_width,
            panel_height,
            "Front Position",
            "T = n t sqrt(g/a)",
            "Z = z/a",
            front_series,
        )
    )
    parts.extend(
        build_panel(
            right_x,
            top_y,
            panel_width,
            panel_height,
            "Top-of-Column Height",
            "tau = t sqrt(g/a)",
            "H = eta/(a n^2)",
            height_series,
        )
    )
    parts.append("</svg>")
    return "\n".join(parts)


def build_series(grouped_points: dict[str, list[Point]]) -> list[Series]:
    palette = ["#0f766e", "#b45309", "#1d4ed8", "#b91c1c", "#7c3aed", "#047857"]
    series_list = []
    for index, a_key in enumerate(sorted(grouped_points, key=float)):
        series_list.append(
            Series(
                label=f"a = {a_key} in",
                points=grouped_points[a_key],
                color=palette[index % len(palette)],
            )
        )
    return series_list


def load_simulation_series(
    csv_path: Path,
    front_time_column: str,
    front_value_column: str,
    height_time_column: str,
    height_value_column: str,
    label: str,
    color: str,
) -> tuple[Series, Series]:
    with csv_path.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        rows = list(reader)

    if not rows:
        raise SystemExit(f"No rows found in simulation CSV: {csv_path}")

    required = [
        front_time_column,
        front_value_column,
        height_time_column,
        height_value_column,
    ]
    missing = [name for name in required if name not in reader.fieldnames]
    if missing:
        raise SystemExit(
            f"Simulation CSV {csv_path} is missing required columns: {', '.join(missing)}"
        )

    front_points = [
        Point(float(row[front_time_column]), float(row[front_value_column])) for row in rows
    ]
    height_points = [
        Point(float(row[height_time_column]), float(row[height_value_column])) for row in rows
    ]
    return (
        Series(label=label, points=front_points, color=color, show_points=False, stroke_width=2.6),
        Series(label=label, points=height_points, color=color, show_points=False, stroke_width=2.6),
    )


def resolve_simulation_option(
    values: list[str], count: int, default: str, option_name: str
) -> list[str]:
    if count == 0:
        return []
    if not values:
        return [default] * count
    if len(values) == 1:
        return values * count
    if len(values) != count:
        raise SystemExit(
            f"{option_name} received {len(values)} values for {count} simulation overlays."
        )
    return values


def resolve_simulation_labels(values: list[str], simulation_paths: list[Path]) -> list[str]:
    if not simulation_paths:
        return []
    if not values:
        return [path.stem for path in simulation_paths]
    if len(values) != len(simulation_paths):
        raise SystemExit(
            f"--simulation-label received {len(values)} values for {len(simulation_paths)} simulation overlays."
        )
    return values


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Plot original Martin-Moyce rectangular paper data from a local .xlsx workbook."
    )
    parser.add_argument(
        "--xlsx",
        default="/Users/chowr/Work/projects/single_phase_vof/martin_moyce_rectangular_digitized.xlsx",
        help="Path to the digitized Martin-Moyce workbook.",
    )
    parser.add_argument(
        "--n-squared",
        type=int,
        default=2,
        help="Rectangular paper series to plot, selected by n^2.",
    )
    parser.add_argument(
        "--front-a",
        default="2.25",
        help='Front-position series to plot by "a" in inches, or "all".',
    )
    parser.add_argument(
        "--height-a",
        default="2.25",
        help='Top-height series to plot by "a" in inches, or "all".',
    )
    parser.add_argument(
        "--output",
        default="martin_moyce_rectangular_paper_data.svg",
        help="Output SVG path.",
    )
    parser.add_argument(
        "--simulation-csv",
        action="append",
        default=[],
        help="Optional simulation CSV to overlay on the paper data plot. Repeat to add multiple overlays.",
    )
    parser.add_argument(
        "--simulation-label",
        action="append",
        default=[],
        help="Legend label for each simulation overlay, in the same order as --simulation-csv.",
    )
    parser.add_argument(
        "--simulation-front-time-column",
        action="append",
        default=[],
        help="Simulation CSV column for front-position time in paper variables.",
    )
    parser.add_argument(
        "--simulation-front-value-column",
        action="append",
        default=[],
        help="Simulation CSV column for front-position value in paper variables.",
    )
    parser.add_argument(
        "--simulation-height-time-column",
        action="append",
        default=[],
        help="Simulation CSV column for top-height time in paper variables.",
    )
    parser.add_argument(
        "--simulation-height-value-column",
        action="append",
        default=[],
        help="Simulation CSV column for top-height value in paper variables.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    workbook_path = Path(args.xlsx)
    if not workbook_path.is_file():
        raise SystemExit(f"Workbook not found: {workbook_path}")

    front_rows, height_rows = load_workbook_tables(workbook_path)
    front_grouped = filter_rows(front_rows, args.n_squared, args.front_a, "T", "Z_front")
    height_grouped = filter_rows(
        height_rows, args.n_squared, args.height_a, "tau", "H_residual_height"
    )

    if not front_grouped:
        raise SystemExit(
            f"No front-position data found for n^2={args.n_squared}, a={args.front_a}."
        )
    if not height_grouped:
        raise SystemExit(
            f"No top-height data found for n^2={args.n_squared}, a={args.height_a}."
        )

    front_series = build_series(front_grouped)
    height_series = build_series(height_grouped)

    simulation_paths = [Path(path) for path in args.simulation_csv]
    simulation_labels = resolve_simulation_labels(args.simulation_label, simulation_paths)
    simulation_front_time_columns = resolve_simulation_option(
        args.simulation_front_time_column,
        len(simulation_paths),
        "paper_T",
        "--simulation-front-time-column",
    )
    simulation_front_value_columns = resolve_simulation_option(
        args.simulation_front_value_column,
        len(simulation_paths),
        "paper_front_Z",
        "--simulation-front-value-column",
    )
    simulation_height_time_columns = resolve_simulation_option(
        args.simulation_height_time_column,
        len(simulation_paths),
        "paper_tau",
        "--simulation-height-time-column",
    )
    simulation_height_value_columns = resolve_simulation_option(
        args.simulation_height_value_column,
        len(simulation_paths),
        "paper_top_H",
        "--simulation-height-value-column",
    )

    simulation_palette = ["#1d4ed8", "#dc2626", "#7c3aed", "#0f766e"]
    simulation_front_series: list[Series] = []
    simulation_height_series: list[Series] = []
    for index, csv_path in enumerate(simulation_paths):
        simulation_front, simulation_height = load_simulation_series(
            csv_path,
            simulation_front_time_columns[index],
            simulation_front_value_columns[index],
            simulation_height_time_columns[index],
            simulation_height_value_columns[index],
            simulation_labels[index],
            simulation_palette[index % len(simulation_palette)],
        )
        simulation_front_series.append(simulation_front)
        simulation_height_series.append(simulation_height)

    if simulation_front_series:
        front_series = simulation_front_series + front_series
        height_series = simulation_height_series + height_series

    title = f"Martin-Moyce Rectangular Paper Data (n^2 = {args.n_squared})"
    svg = make_svg(front_series, height_series, title)

    output_path = Path(args.output)
    output_path.write_text(svg, encoding="utf-8")
    print(output_path.resolve())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
