# /*************************************************
import sys

sys.path.append("../")

import editorial_settings
import matplotlib as mpl
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import glob
import os
import re
from matplotlib import gridspec
from matplotlib.ticker import MaxNLocator, MultipleLocator
from scipy.interpolate import make_interp_spline

# mpl.rcParams['figure.figsize'] = [4.25, 5.]

from itertools import cycle

marker_list = ["o", "D", "s", "^", "v"]

sec_to_hr = 1.0 / (60 * 60)
Pa_to_MPa = 1e-6
cjam = plt.cm.binary(np.linspace(1, 0, 5))

# def analytical_solution(r, r0=1, k=1, u0=300, beta=0.1, q= 1200):
#    return u0 + (q*np.power(r0,2)/(6*k)) * ( (1-np.power((r/r0),2)) - 0.3*beta*(1-np.power((r/r0),4)) )


def main():
    working_dir = os.path.abspath(".")
    folder_paths = glob.glob("group*/**/")
    folder_paths = [f for f in folder_paths if f != "group_A/7/"]

    file_paths = glob.glob(os.path.join(working_dir, "*_out.csv"))

    for folder in folder_paths:
        testno = folder.replace("/", "_")

        fig = plt.figure()
        gs = gridspec.GridSpec(1, 1)
        ax = fig.add_subplot(gs[0])

        markers = cycle(marker_list)

        """
        NAFEMS reference solution
        """
        nafemsfile = folder + "reference_solution.csv"
        if os.path.exists(nafemsfile):
            nafems = pd.read_csv(nafemsfile)

            cols = nafems.columns[nafems.columns != "t"].tolist()
            has_sigma = any("sigma" in col for col in cols)
            has_epsilon = any("epsilon" in col for col in cols)
            has_gamma = any("gamma" in col for col in cols)

            ax.plot([], [], "none", label=r"NAFEMS")
            for qoi in cols:
                ax.plot(
                    nafems["t"], nafems[qoi], next(markers), mfc="none", label=qoi
                )  # +" (NAFEMS)")

        if has_sigma:
            for i in range(4 - len(cols)):
                ax.plot([], [], "none", label=" ")

        if has_epsilon:
            for i in range(3 - len(cols)):
                ax.plot([], [], "none", label=" ")

        # ymin, ymax= ax.get_ylim()
        # ax.set_ylim(ymin, ymax)

        test = testno.split("_", 2)[2]
        outputfile = folder + test + "out.csv"
        print(outputfile)

        """
         ANALYTIC SOLUTIONS
         """
        ax.set_prop_cycle(None)

        ncoltwo = True

        if os.path.exists(outputfile):
            bison = pd.read_csv(outputfile)
            bcols = bison.columns[
                ~bison.columns.str.contains("avg", case=False, na=False)
                & ~bison.columns.str.contains("time", case=False, na=False)
            ]  # .tolist()
            # print(bcols)

            if not bcols.empty:
                ax.plot([], [], "none", label=r"Analytic")
                for qoi in bcols:
                    qoitag = qoi
                    if qoi == "exact_stress_xx":
                        qoitag = r"$\sigma_{xx}$"  # (Analytic)'
                    if qoi == "exact_stress_yy":
                        qoitag = r"$\sigma_{yy}$"  # (Analytic)'
                    if qoi == "exact_stress_zz":
                        qoitag = r"$\sigma_{zz}$"  # (Analytic)'
                    if qoi == "exact_stress_eff":
                        qoitag = r"$\sigma_{eff}$"  # (Analytic)'
                    if qoi == "exact_creep_xx_SH":
                        qoitag = r"S.H.: $\varepsilon_{xx}^c$"  # (Analytic)'
                    if qoi == "exact_creep_yy_SH":
                        qoitag = r"S.H.: $\varepsilon_{yy}^c$"  # (Analytic)'
                    if qoi == "exact_creep_xx_TH":
                        qoitag = r"T.H.: $\varepsilon_{xx}^c$"  # (Analytic)'
                    if qoi == "exact_creep_yy_TH":
                        qoitag = r"T.H.: $\varepsilon_{yy}^c$"  # (Analytic)'
                    if qoi == "exact_creep_xx":
                        qoitag = r"$\varepsilon_{xx}^c$"  # (Analytic)'
                    if qoi == "exact_creep_yy":
                        qoitag = r"$\varepsilon_{yy}^c$"  # (Analytic)'
                    if qoi == "exact_creep_zz":
                        qoitag = r"$\varepsilon_{zz}^c$"  # (Analytic)'
                    if qoi == "exact_creep_eff":
                        qoitag = r"$\varepsilon_{eff}^c$"  # (Analytic)'
                    if qoi == "exact_creep_gamma_xy":
                        qoitag = r"$\gamma_{xy}^c$"  # (Analytic)'

                    ax.plot(
                        bison["time"],
                        bison[qoi],
                        "-",
                        dashes=[1, 1],
                        lw=3.5,
                        label=qoitag,
                    )

                if has_sigma:
                    ncoltwo = False
                    for i in range(4 - len(bcols)):
                        ax.plot([], [], "none", label=" ")

                if has_epsilon:
                    ncoltwo = False
                    for i in range(3 - len(bcols)):
                        ax.plot([], [], "none", label=" ")

                if has_gamma:
                    ncoltwo = False
                    for i in range(1 - len(bcols)):
                        ax.plot([], [], "none", label=" ")

        """
         NUMERICAL SOLUTIONS
         """
        ax.set_prop_cycle(None)

        if os.path.exists(outputfile):
            bison = pd.read_csv(outputfile)
            bcols = bison.columns[
                ~bison.columns.str.contains("exact", case=False, na=False)
                & ~bison.columns.str.contains("time", case=False, na=False)
            ].tolist()
            # bcols= bison.columns[bison.columns != "time"].tolist()

            ax.plot([], [], "none", label=r"MOOSE")
            lwdx = 0.0
            for qoi in bcols:

                qoitag = qoi
                if qoi == "avg_stress_xx":
                    qoitag = r"$\sigma_{xx}$"  # (MOOSE)'
                if qoi == "avg_stress_yy":
                    qoitag = r"$\sigma_{yy}$"  # (MOOSE)'
                if qoi == "avg_stress_zz":
                    qoitag = r"$\sigma_{zz}$"  # (MOOSE)'
                if qoi == "avg_stress_eff":
                    qoitag = r"$\sigma_{eff}$"  # (MOOSE)'
                if qoi == "avg_creep_gamma_xy":
                    qoitag = r"$\gamma_{xy}^c$"  # (MOOSE)'
                if qoi == "avg_creep_xx":
                    qoitag = r"$\varepsilon_{xx}^c$"  # (MOOSE)'
                if qoi == "avg_creep_yy":
                    qoitag = r"$\varepsilon_{yy}^c$"  # (MOOSE)'
                if qoi == "avg_creep_zz":
                    qoitag = r"$\varepsilon_{zz}^c$"  # (MOOSE)'
                if qoi == "avg_creep_eff":
                    qoitag = r"$\varepsilon_{eff}^c$"  # (MOOSE)'

                ax.plot(
                    bison["time"],
                    bison[qoi],
                    "-",
                    lw=3.0 - 1 * (lwdx % 2),
                    zorder=20 + 10 * (lwdx % 2),
                    label=qoitag,
                )
                lwdx += 1  # 0.25

        ax.set_xlabel(r"Time (hours)")

        ax.xaxis.set_major_locator(MaxNLocator(nbins=5))
        ax.yaxis.set_major_locator(MaxNLocator(nbins=5))

        #      ax.plot([-100,1100],[0,0],'-',c='dimgray',lw=1)

        editorial_settings.putLegendLines(ax)

        if has_sigma:
            ax.set_ylabel(r"Stress (N/mm$^2$)")
        if has_epsilon:
            ax.set_ylabel(r"Creep strain (-)")
            ymin, ymax = ax.get_ylim()
            ax.set_ylim(ymin, ymax * 1.325)
            # ordered_labels= [r'$\varepsilon_{xx}^c$', r'$\varepsilon_{xx}^c$', r'$\varepsilon_{xx}^c$', r'$\varepsilon_{xx}^c$']
        if has_gamma:
            ax.set_ylabel(r"Creep strain (-)")

        if ncoltwo:
            ncolx = 2
        else:
            ncolx = 3
        leg = ax.legend(
            loc="upper center",
            # bbox_to_anchor=(0.5,0.98),
            ncol=ncolx,
            fontsize=9,
        )  # , loc='upper left')#loc='center left', bbox_to_anchor=(1.05, 0.5))

        # leg.get_texts()[0].set_fontsize(8)
        # leg.get_texts()[4].set_fontsize(8)
        # leg.get_texts()[8].set_fontsize(8)
        # # leg.get_texts()[0].set_weight('italic')

        plt.savefig(testno + "plot.pdf", bbox_inches="tight")
        plt.clf()
        plt.close()

    folder = "group_A/7/"
    testno = folder.replace("/", "_")

    fig = plt.figure()
    gs = gridspec.GridSpec(1, 1)
    ax = fig.add_subplot(gs[0])

    ax.set_prop_cycle(None)

    outputfile = folder + "7_elastic_profile_stress_profile_FINAL.csv"
    if os.path.exists(outputfile):
        bison = pd.read_csv(outputfile)
        # bison= bison[ (bison['y']==12.5) & (bison['z']==0)]
        # bcols= bison.columns[bison.columns != "time"].tolist()
        bcols = [col for col in bison.columns if col not in ["x", "y", "z", "id"]]
        print(bcols)
        print(bison)

        ax.plot([], [], "none", label=r"MOOSE")
        for qoi in bcols:
            qoitag = qoi
            if qoi == "sigma_rr_recovered":
                qoitag = r"$\sigma_{rr}$"
            if qoi == "sigma_hoop_recovered":
                qoitag = r"$\sigma_{\theta\theta}$"
            if qoi == "sigma_zz_recovered":
                qoitag = r"$\sigma_{zz}$"
            ax.plot(
                bison["x"],
                bison[qoi],
                "--",
                lw=2.5,
                alpha=0.5,
                label=r"elastic: " + qoitag,
            )

    # plt.savefig(testno+'_elastic_plot.pdf',bbox_inches='tight')
    # plt.clf()

    # fig = plt.figure()
    # gs = gridspec.GridSpec(1,1)
    # ax = fig.add_subplot(gs[0])

    ax.set_prop_cycle(None)

    nafemsfile = folder + "reference_solution.csv"
    if os.path.exists(nafemsfile):
        nafems = pd.read_csv(nafemsfile)

        ax.plot([], [], "none", label=r"NAFEMS")
        cols = nafems.columns[nafems.columns != "r"].tolist()
        for qoi in cols:
            ax.plot(nafems["r"], nafems[qoi], next(markers), mfc="none", label=qoi)
    ax.plot([], [], "none", label=" ")

    ax.set_xlim(ax.get_xlim())
    # ax.set_ylim(ax.get_ylim())

    # analytical_solution

    ax.set_prop_cycle(None)

    ax.plot([], [], "none", label=r"Analytic")
    radii = np.arange(100, 200, 1)
    ax.plot(
        radii,
        625.96 * (1 - 0.6 * (200 / radii) ** 0.4),
        "-",
        dashes=[1, 1],
        lw=3.5,
        label=r"SS: $\sigma_{\theta\theta}$",
    )
    ax.plot(
        radii,
        625.96 * (1 - (200 / radii) ** 0.4),
        "-",
        dashes=[1, 1],
        lw=3.5,
        label=r"SS: $\sigma_{rr}$",
    )
    ax.plot(
        radii,
        625.96 * (1 - 0.8 * (200 / radii) ** 0.4),
        "-",
        dashes=[1, 1],
        lw=3.5,
        label=r"SS: $\sigma_{zz}$",
    )

    ax.set_prop_cycle(None)

    outputfile = folder + "7_inelastic_profile_stress_profile_FINAL.csv"
    if os.path.exists(outputfile):
        bison = pd.read_csv(outputfile)
        bison = bison[(bison["y"] == 12.5) & (bison["z"] == 0)]
        # bcols= bison.columns[bison.columns != "time"].tolist()
        bcols = [col for col in bison.columns if col not in ["x", "y", "z", "id"]]
        print(bcols)
        print(bison)

        ax.plot([], [], "none", label=r"MOOSE")
        for qoi in bcols:
            qoitag = qoi
            if qoi == "sigma_rr_recovered":
                qoitag = r"$\sigma_{rr}$"
            if qoi == "sigma_hoop_recovered":
                qoitag = r"$\sigma_{\theta\theta}$"
            if qoi == "sigma_zz_recovered":
                qoitag = r"$\sigma_{zz}$"
            ax.plot(bison["x"], bison[qoi], "-", lw=2.5, label=r"SS: " + qoitag)

    ax.set_ylabel(r"Stress (N/mm$^2$)")
    ax.set_xlabel(r"Radius (mm)")

    ax.xaxis.set_major_locator(MaxNLocator(nbins=5))
    ax.yaxis.set_major_locator(MaxNLocator(nbins=5))

    editorial_settings.putLegendLines(ax)

    ymin, ymax = ax.get_ylim()
    ax.set_ylim(ymin, ymax * 1.25)
    plt.legend(
        loc="upper center", ncol=4, fontsize=7
    )  # loc='center left', bbox_to_anchor=(1.05, 0.5))

    plt.savefig(testno + "plot.pdf", bbox_inches="tight")
    plt.clf()
    plt.close()


if __name__ == "__main__":
    main()
