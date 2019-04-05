try:
    import sympy

except ImportError:
    print("The 'mms' package requires sympy, it can be installed by running " \
          "`pip install sympy --user`.")

else:
    from fparser import FParserPrinter, fparser, print_fparser
    from moosefunction import MooseFunctionPrinter, moosefunction, print_moose
    from evaluate import evaluate


try:
    import matplotlib

except ImportError:
    print("The 'mms' package requires matplotlib, it can be installed by running " \
          "`pip install matplotlib --user`.")

else:
    from ConvergencePlot import ConvergencePlot, plot
