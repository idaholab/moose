import matplotlib.pyplot as plt
import sys

f = open("gold/ex01_aquitard_extraction_drawdown_0004.csv", "r")
aq = [list(map(float, line.split(","))) for line in f.readlines()[1:]]
f.close()

f = open("gold/ex01_lower_extraction_drawdown_0004.csv", "r")
lo = [list(map(float, line.split(","))) for line in f.readlines()[1:]]
f.close()

plt.figure()
plt.plot([x[0] for x in lo], [x[4] for x in lo], 'b-', label = 'borehole in lower aquifer')
plt.plot([x[0] for x in aq], [x[4] for x in aq], 'r-', label = 'borehole in aquitard')
plt.legend()
plt.ylabel("Head change (m)")
plt.xlabel("x (m)")
plt.title("Drawdown in upper aquifer")
plt.savefig("../../doc/content/media/porous_flow/groundwater_ex01.png")
plt.show()

sys.exit(0)
