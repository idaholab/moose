import matplotlib.pyplot as plt
import pandas

data = pandas.read_csv('master_out.csv')

plt.plot(data['time'], data['area'])
plt.show()
