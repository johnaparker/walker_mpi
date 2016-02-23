import h5py
import numpy as np
import pylab as py
import matplotlib.animation as animation

xc = [0,5,0,5]
yc = [0,0,5,5]

def get_data(filename, num_walkers = 4):
    data_f = h5py.File("test.h5")
    walker1_d = data_f["0"]
    T = walker1_d.shape[1]

    data = np.zeros((num_walkers,2,T))

    for i in range(num_walkers):
        walker_d = data_f[str(i)]
        walker_a = np.array(walker_d)
        data[i] = walker_a

    return data

def plot_trajectories(data, Tf = None):
    py.xlim([-.5,9.5])
    py.ylim([-.5,9.5])
    py.axvline(xc[1]-.5, color='black', linestyle="--")
    py.axhline(yc[2]-.5, color='black', linestyle="--")
    if Tf == None:
        Tf = -1
    for i in range(num_walkers):
        x = data[i,0,:Tf] 
        y = data[i,1,:Tf]
        py.plot(x,y)
        py.plot(x[0],y[0],'bo')
        py.plot(x[-1],y[-1],'b*')


def video(data, Tf = None):
    fig, ax = py.subplots()
    py.xlim([-.5,9.5])
    py.ylim([-.5,9.5])
    py.axvline(xc[1]-.5, color='black', linestyle="--")
    py.axhline(yc[2]-.5, color='black', linestyle="--")

    if Tf == None:
        Tf = data.shape[2]

    lines = []
    for i in range(num_walkers):
        x = data[i,0,:1] 
        y = data[i,1,:1]
        line, = py.plot(x,y, 'o')
        lines.append(line)

    def animate(i): 
        for j,line in enumerate(lines):
            line.set_xdata(data[j,0,i-1:i])
            line.set_ydata(data[j,1,i-1:i])
        return lines
    ani = animation.FuncAnimation(fig, animate, np.arange(1,Tf), interval=300,
            blit=True)
    
    py.show()

    


if __name__ == "__main__":
    num_walkers = 16
    filename = "test.h5"
    data = get_data(filename, num_walkers)
    plot_trajectories(data,20)
    video(data)
