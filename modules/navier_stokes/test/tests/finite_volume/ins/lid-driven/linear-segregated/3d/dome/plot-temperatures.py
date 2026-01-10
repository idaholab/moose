import argparse
import pandas as pd
import matplotlib.pyplot as plt

def comma_separated_list(value):
    """Helper to split comma-separated input into a list"""
    items = [v.strip() for v in value.split(",") if v.strip()]
    if not items:
        raise argparse.ArgumentTypeError("Expected one or more comma-separated values.")
    return items

def main():
    parser = argparse.ArgumentParser(
        description="Plot CSV data with time on the x-axis for multiple cases."
    )
    parser.add_argument(
        "--csv-files", required=True, type=comma_separated_list,
        help="Comma-separated list of CSV file paths, e.g. 'run1.csv,run2.csv'"
    )
    parser.add_argument(
        "--cases", required=True, type=comma_separated_list,
        help="Comma-separated list of case labels matching the CSV files, e.g. 'caseA,caseB'"
    )
    args = parser.parse_args()

    if len(args.csv_files) != len(args.cases):
        raise ValueError(
            f"--cases count ({len(args.cases)}) must match number of CSV files ({len(args.csv_files)})."
        )

    temperature_columns = ["T_avg", "T_boundary_max", "T_max"]

    for csv_path, case in zip(args.csv_files, args.cases):
        # Read CSV
        df = pd.read_csv(csv_path)

        # Validate required columns
        missing = [c for c in ["time", *temperature_columns] if c not in df.columns]
        if missing:
            raise ValueError(f"{csv_path}: missing required columns: {missing}")

        # Enforce strictly numeric time
        if not pd.api.types.is_numeric_dtype(df["time"]):
            try:
                df["time"] = df["time"].astype(float)
            except ValueError as e:
                bad_vals = df[
                    ~df["time"].astype(str).str.replace(".", "", 1).str.isdigit()
                ]["time"].unique()
                raise ValueError(
                    f"{csv_path}: non-numeric values found in 'time': {bad_vals}"
                ) from e

        # Drop the first time point
        df = df.iloc[1:, :]

        # Plot temperature columns with case appended to labels
        time = df["time"]
        for col in temperature_columns:
            plt.plot(time, df[col], label=f"{col} ({case})")

    plt.xlabel("time (s)")
    plt.ylabel("Temperature (K)")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()
