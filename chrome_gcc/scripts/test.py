import matplotlib.pyplot as plt
import numpy as np

def plotAndSave(x,y,z,title,xlabel,ylabel,legend,save_fig_name):
    fig, ax = plt.subplots(1, 1)
    ax.plot(x, y, label='estimate')
    ax.plot(x, z, label='true')
    ax.set_title(title, fontsize=12, color='r')
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.legend(legend)
    plt.savefig(save_fig_name)
    pass

if __name__=="__main__":
    x = np.array([1,2])
    y = np.array([10,20])
    z = np.array([20,30])

    plotAndSave(x,y,z,"bandwidth estimate","time(s)","kbit/s",["bbr","true"],'bw.png')

