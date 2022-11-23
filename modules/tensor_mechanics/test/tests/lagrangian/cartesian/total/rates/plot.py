#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

if __name__ == "__main__":
    G = 5000  # shear modulus

    def analytical_truesdell(k):
        s11 = k * k
        s22 = k * 0
        s12 = k
        return s11, s12, s22

    def analytical_jaumann(k):
        s11 = -np.cos(k) + 1
        s22 = -s11
        s12 = np.sin(k)
        return s11, s12, s22

    def analytical_green_naghdi(k):
        b = np.arctan(k / 2)
        s11 = 4 * (np.cos(2 * b) * np.log(np.cos(b)) + b *
                   np.sin(2 * b) - np.sin(b) * np.sin(b))
        s22 = -s11
        s12 = 2 * np.cos(2 * b) * (2 * b - 2 * np.tan(2 * b)
                                   * np.log(np.cos(b)) - np.tan(b))
        return s11, s12, s22

    def compare(rate_name, csv, analytical):
        numerical_sol = np.loadtxt(csv, skiprows=1, delimiter=',')
        k = numerical_sol[:, 0]
        s11, s12, s22 = analytical(k)
        fig, (ax11, ax12, ax22) = plt.subplots(
            nrows=1, ncols=3, figsize=(16, 6), dpi=80)

        ax11.plot(k, s11, 'k-', label="analytical")
        ax11.plot(k, numerical_sol[:, 1], 'r--', label="numerical")
        ax11.set_xlabel("u_x / L")
        ax11.set_ylabel("\sigma_{xx}")
        ax11.legend(loc='best')

        ax12.plot(k, s12, 'k-', label="analytical")
        ax12.plot(k, numerical_sol[:, 2], 'r--', label="numerical")
        ax12.set_xlabel("u_x / L")
        ax12.set_ylabel("\sigma_{xy}")
        ax12.legend(loc='best')

        ax22.plot(k, s22, 'k-', label="analytical")
        ax22.plot(k, numerical_sol[:, 3], 'r--', label="numerical")
        ax22.set_xlabel("u_x / L")
        ax22.set_ylabel("\sigma_{yy}")
        ax22.legend(loc='best')

        fig.suptitle(rate_name)
        fig.tight_layout()
        fig.savefig(rate_name + ".png")
        plt.draw()

    compare("Truesdell rate of Cauchy stress",
            "truesdell_shear_out.csv", analytical_truesdell)

    compare("Jaumann rate of Cauchy stress",
            "jaumann_shear_out.csv", analytical_jaumann)

    compare("Green-Naghdi rate of Cauchy stress",
            "green_naghdi_shear_out.csv", analytical_green_naghdi)

    plt.show()
