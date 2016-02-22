import h5py
import numpy as np
import pylab as py

num_walkers = 4
xc = [0,5,0,5]
yc = [0,0,5,5]

data_f = h5py.File("test.h5")
walker1_d = data_f["0"]
T = walker1_d.shape[1]

data = np.zeros((num_walkers,2,T))

for i in range(num_walkers):
    walker_d = data_f[str(i)]
    walker_a = np.array(walker_d)
    data[i] = walker_a

py.xlim([-.5,9.5])
py.ylim([-.5,9.5])
py.axvline(xc[1]-.5, color='black', linestyle="--")
py.axhline(yc[2]-.5, color='black', linestyle="--")
for i in range(num_walkers):
    x = data[i,0,:] + xc[i]
    y = data[i,1,:] + yc[i]
    py.plot(x[0],y[0],'bo')
    py.plot(x[-1],y[-1],'b*')
    py.plot(x,y)

py.show()
