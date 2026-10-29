import matplotlib as mpl
import numpy as np

# mpl.rcParams['pdf.use14corefonts'] = True
# mpl.rcParams['ps.useafm'] = True
# mpl.rcParams['text.usetex'] = True

mpl.rcParams["axes.axisbelow"] = True
mpl.rcParams["axes.grid"] = False
mpl.rcParams["axes.titlesize"] = 13
mpl.rcParams["axes.labelsize"] = 11
mpl.rcParams["axes.edgecolor"] = "555555"
mpl.rcParams["axes.labelcolor"] = "dimgray"
mpl.rcParams["axes.formatter.useoffset"] = True
mpl.rcParams["axes.formatter.use_mathtext"] = True

mpl.rcParams["font.size"] = 11
mpl.rcParams["lines.linewidth"] = 3.0  # 2.0
mpl.rcParams["lines.markersize"] = 6
mpl.rcParams["lines.markeredgewidth"] = 1.5
# mpl.rcParams['figure.titlesize'] = 'large'
mpl.rcParams["axes.titlepad"] = 10.0  # pad between axes and title in points
# mpl.rcParams['axes.labelpad'] = 10.0
mpl.rcParams["axes.labelpad"] = 2.0

mpl.rcParams["hatch.linewidth"] = 0.5

mpl.rcParams["grid.color"] = "D4D4D4"  #'b0b0b0' #whitesmoke' # 'gainsboro'
mpl.rcParams["grid.linestyle"] = ":"
mpl.rcParams["grid.linewidth"] = 0.25

mpl.rcParams["figure.figsize"] = [4.25, 5.0]
mpl.rcParams["figure.dpi"] = 80
mpl.rcParams["savefig.dpi"] = 100

# mpl.rcParams['errorbar.capsize'] = 3

mpl.rcParams["legend.frameon"] = False
mpl.rcParams["legend.fancybox"] = False
mpl.rcParams["legend.loc"] = "best"
mpl.rcParams["legend.numpoints"] = 1
mpl.rcParams["legend.fontsize"] = 9
mpl.rcParams["legend.framealpha"] = None
mpl.rcParams["legend.scatterpoints"] = 1
mpl.rcParams["legend.edgecolor"] = "inherit"
mpl.rcParams["legend.handletextpad"] = 0.05
# mpl.rcParams['legend.columnspacing'] = 0.1
mpl.rcParams["legend.columnspacing"] = 1.0
mpl.rcParams["legend.labelspacing"] = 0  # .5

mpl.rcParams["xtick.direction"] = "in"
mpl.rcParams["xtick.color"] = "555555"
mpl.rcParams["ytick.direction"] = "in"
mpl.rcParams["ytick.color"] = "555555"
mpl.rcParams["xtick.top"] = True
mpl.rcParams["ytick.right"] = True

mpl.rcParams["savefig.transparent"] = False


def putLegendLines(ax, gridlinecolor="silver"):
    ax.grid(True, which="major", ls="--", c=gridlinecolor, lw=0.5)
    ax.grid(True, which="minor", ls=":", c=gridlinecolor, lw=0.5)
    ax.minorticks_on()
