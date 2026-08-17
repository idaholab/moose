import pandas as pd
import matplotlib.pyplot as plt


def plot_gp_predictions(
    csv_file,
    error_scale=1.0,
    save_path=None
):
    """
    Read a CSV file and plot true values, GP mean predictions,
    and GP uncertainty bounds.

    Expected CSV columns:
        x
        y_true
        GP_avg
        GP_avg_std

    Parameters
    ----------
    csv_file : str
        Path to the CSV file.

    error_scale : float
        Multiplier for the standard deviation.
        Use 1.0 for ±1 standard deviation.
        Use 1.96 for approximately 95% confidence bounds.

    save_path : str or None
        If provided, saves the figure to this path.
    """

    # Read CSV
    df = pd.read_csv(csv_file)

    # Check required columns
    required_columns = ["x", "y_true", "GP_avg", "GP_avg_std"]
    missing = [col for col in required_columns if col not in df.columns]

    if missing:
        raise ValueError(f"Missing required columns: {missing}")

    # Sort by x for cleaner plotting
    df = df.sort_values("x")

    x = df["x"]
    y_true = df["y_true"]
    gp_mean = df["GP_avg"]
    gp_std = df["GP_avg_std"]

    lower_bound = gp_mean - error_scale * gp_std
    upper_bound = gp_mean + error_scale * gp_std

    # Plot
    plt.figure(figsize=(8, 5))

    plt.plot(x, y_true, "k-", label="True values")
    plt.plot(x, gp_mean, "b-", label="GP mean prediction")

    plt.fill_between(
        x,
        lower_bound,
        upper_bound,
        alpha=0.25,
        label=f"GP error bounds ±{error_scale} std"
    )

    plt.xlabel("x")
    plt.ylabel("Output")
    plt.title("Gaussian Process Predictions vs True Values")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()

    if save_path is not None:
        plt.savefig(save_path, dpi=300)

    plt.show()


plot_gp_predictions("Test_data_penalty_3.csv", error_scale=1.96) # 95% confidence bounds