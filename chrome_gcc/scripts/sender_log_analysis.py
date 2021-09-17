import matplotlib.pyplot as plt
import numpy as np
import argparse
import re

int64max = 9223370000000000.0

def isUsefullLine(line):
    split = line.split(' ')
    if (split[0] not in ["I0831", "I0830"]):# I means Info, 08 31 is date
        return False
    return True

def parseTime(split):
    time = 0
    # time_split = re.split(':|\.',time_str)
    hour = int(split[1])
    min = int(split[2])
    sec_us = split[3].split('.')
    sec = int(sec_us[0])
    us = int(sec_us[1])
    time = us+sec*1000000+min*60*1000000+hour*60*60*1000000
    return time

def findElementIdx(list_like,target):
    for i,ele in enumerate(list_like):
        if(ele==target):
            return i
    return -1

def parsePacingRate(line):
    ack_rate = -1
    idx = findElementIdx(line,'Pacing_rate')
    if(idx>-1):
        ack_rate = float(line[idx+2])
    return ack_rate
def parseReceiverLimit(line):
    limit = -1
    idx = findElementIdx(line,'Receiver_limit')
    if(idx>-1):
        limit = float(line[idx+2])
    return limit
def parseLossRate(line):
    limit = -1
    idx = findElementIdx(line,'%')
    if(idx>-1):
        limit = float(line[idx-1])
    return limit

def parseLine(line):
    key_value = {}
    split = re.split(' |:|\n',line)
    key_value["time"] = parseTime(split)
    key_value["Pacing_rate"] = parsePacingRate(split)
    key_value["Receiver_limit"] = parseReceiverLimit(split)
    key_value["Loss_rate"] = parseLossRate(split)
    return key_value

def readFile(file_path):
    f = open(file_path, 'r')
    lines = f.readlines()

    rate_val = []
    limit_val =[]
    loss_rate_val =[]
    time_val = []
    for line in lines:
        if (isUsefullLine(line)):
            key_value = parseLine(line)
            rate_val.append(key_value["Pacing_rate"])
            limit_val.append(key_value["Receiver_limit"])
            loss_rate_val.append(key_value["Loss_rate"])
            time_val.append(key_value["time"])
        pass
    rate_val = np.array(rate_val)
    loss_rate_val = np.array(loss_rate_val)
    loss_rate_val[loss_rate_val>100] = 0
    limit_val = np.array(limit_val)
    limit_val[limit_val==int64max] = 0
    time_val = np.array(time_val)
    time_val = (time_val - time_val[0])/1000000
    return time_val,rate_val,limit_val,loss_rate_val

def plotAndSave(x, y, title, xlabel, ylabel, legend, save_fig_name, z=None):
    fig, ax = plt.subplots(1, 1)
    ax.plot(x, y, label='estimate')
    if z is not  None:
        ax.plot(x, z, label='true')
    ax.set_title(title, fontsize=12, color='r')
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.legend(legend)
    plt.savefig(save_fig_name)
    pass
def multiPlotAndSave(xs, ys, title, xlabel, ylabel, legend, save_fig_name,type="plot"):
    fig, ax = plt.subplots(1, 1)
    for i,y in enumerate(ys):
        if type == "plot":
            ax.plot(xs[i], ys[i])
        if type == "scatter":
            ax.scatter(xs[i], ys[i],s=1,c='r')
    ax.set_title(title, fontsize=12, color='r')
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.legend(legend)
    plt.savefig(save_fig_name)
    pass

def getBandwidth1():
    time = np.arange(350)
    bandwidth = np.zeros_like(time)
    b500 = 500*np.ones(50)
    b1000 = 1000*np.ones(50)
    b1500 = 1500*np.ones(50)
    b2000 = 2000*np.ones(50)

    bandwidth = np.hstack([b500,b1000,b1500,b2000,b1500,b1000,b500])
    return  time, bandwidth

def getBandwidth():
    time = np.array([])
    bandwidth = np.array([])

    time,bandwidth = getBandwidth1()

    return  time,bandwidth

if __name__ == "__main__":
    # test2()
    # test3()
    parser = argparse.ArgumentParser(
        description='Input the log file you want to analysis, default directory is current directory')
    parser.add_argument('--log_filename', dest='log_filename', help='log filename you want to parse')
    parser.add_argument('--log_dir', dest='dir', default=".", help='directory of log file')
    args = parser.parse_args()
    # print(args.log_filename)
    # print(args.dir)
    log_filename = args.log_filename
    log_dir = args.dir
    file = log_dir + log_filename

    x_log,pr,rl,loss_rate = readFile(file)
    filename = log_filename.split('.')[0]
    # plotAndSave(x_log,pr,"sender estimate","time(s)","value(kbit/s)",["pacing rate","receiver limit"],filename+'.png',z=rl)

    x_bandwidth, bandwidth_true = getBandwidth()
    ys = [pr,rl,bandwidth_true]
    xs = [x_log,x_log,x_bandwidth]
    multiPlotAndSave(xs,ys,"sender estimate","time(s)","value(kbit/s)",["pacing rate","receiver limit","true bandwidth"],filename+'_band_estimate.png')


    ys = [loss_rate]
    xs = [x_log]
    multiPlotAndSave(xs,ys,"loss rate","time(s)","loss rate(%)",["loss rate"],filename+'_loss_rate.png',type="scatter")
    print("Generate figure done")

