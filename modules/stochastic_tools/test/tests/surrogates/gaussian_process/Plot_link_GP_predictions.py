import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from statistics import NormalDist


def lognormal_bounds_from_mean_std(mean, std, confidence=0.68):
    """
    Compute asymmetric lognormal prediction bounds from final-space
    mean and standard deviation.

    Parameters
    ----------
    mean : array-like
        Mean of the final wrapped GP output, i.e. GP_avg.

    std : array-like
        Standard deviation of the final wrapped GP output, i.e. GP_avg_std.

    confidence : float
        Central confidence level for the interval.
        Use 0.68 for approximately one-sigma bounds.
        Use 0.95 for approximately 95% bounds.

    Returns
    -------
    lower : ndarray
        Lower lognormal quantile bound. Always positive.

    upper : ndarray
        Upper lognormal quantile bound.
    """

    mean = np.asarray(mean, dtype=float)
    std = np.asarray(std, dtype=float)

    if np.any(mean <= 0):
        raise ValueError(
            "Lognormal bounds require GP_avg > 0. "
            "A lognormal distribution has strictly positive mean."
        )

    if np.any(std < 0):
        raise ValueError("GP_avg_std must be non-negative.")

    # Convert final-space mean and std to lognormal parameters.
    #
    # If Y ~ LogNormal(mu_log, sigma_log^2), then:
    #
    #   E[Y] = exp(mu_log + 0.5 sigma_log^2)
    #   Var[Y] = (exp(sigma_log^2) - 1) exp(2 mu_log + sigma_log^2)
    #
    # Solving for mu_log and sigma_log:
    sigma_log_sq = np.log(1.0 + (std / mean) ** 2)
    sigma_log = np.sqrt(sigma_log_sq)
    mu_log = np.log(mean) - 0.5 * sigma_log_sq

    # Central confidence interval in latent normal space
    alpha = 1.0 - confidence
    z = NormalDist().inv_cdf(1.0 - alpha / 2.0)

    lower = np.exp(mu_log - z * sigma_log)
    upper = np.exp(mu_log + z * sigma_log)

    return lower, upper


def plot_wrapped_gp_predictions(
    csv_file,
    confidence=0.68,
    save_path=None
):
    """
    Read a CSV file and plot true values, wrapped GP mean predictions,
    and asymmetric lognormal uncertainty bounds.

    Expected CSV columns:
        x
        y_true
        GP_avg
        GP_avg_std

    Notes
    -----
    GP_avg is assumed to be the mean of the final wrapped output.

    GP_avg_std is assumed to be the standard deviation of the final
    wrapped output, whose distribution is lognormal.

    Therefore, no transformation is applied to GP_avg itself.
    """

    df = pd.read_csv(csv_file)

    required_columns = ["x", "y_true", "GP_avg", "GP_avg_std"]
    missing = [col for col in required_columns if col not in df.columns]

    if missing:
        raise ValueError(f"Missing required columns: {missing}")

    df = df.sort_values("x")

    x = df["x"].to_numpy()
    y_true = df["y_true"].to_numpy()
    gp_mean = df["GP_avg"].to_numpy()
    gp_std = df["GP_avg_std"].to_numpy()

    lower_bound, upper_bound = lognormal_bounds_from_mean_std(
        gp_mean,
        gp_std,
        confidence=confidence
    )

    plt.figure(figsize=(8, 5))

    plt.plot(x, y_true, "k-", label="True values")
    plt.plot(x, gp_mean, "b-", label="Wrapped GP mean")

    plt.fill_between(
        x,
        lower_bound,
        upper_bound,
        alpha=0.25,
        label=f"{int(confidence * 100)}% lognormal interval"
    )

    plt.xlabel("x")
    plt.ylabel("Output")
    plt.title("Wrapped GP Predictions with Lognormal Error Bounds")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()

    if save_path is not None:
        plt.savefig(save_path, dpi=300)

    plt.show()


# Example usage: approximately one-sigma central interval
plot_wrapped_gp_predictions("Test_data_link.csv", confidence=0.95) # 95% confidence level