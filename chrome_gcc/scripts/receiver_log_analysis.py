import matplotlib.pyplot as plt
import numpy as np
import argparse
import re

def isUsefullLine(line):
    split = line.split(' ')
    if (split[0] != "I0830"):
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

def parseAckRate(line):
    ack_rate = -1
    idx = findElementIdx(line,'Ack_rate')
    if(idx>-1):
        ack_rate = float(line[idx+2])
    return ack_rate

def parseLine(line):
    key_value = {}
    split = re.split(' |:',line)
    key_value["time"] = parseTime(split)
    key_value["Ack_rate"] = parseAckRate(split)

    return key_value

def readFile(file_path):
    f = open(file_path, 'r')
    lines = f.readlines()

    ack_val = []
    time_val = []
    for line in lines:
        if (isUsefullLine(line)):
            key_value = parseLine(line)
            ack_val.append(key_value["Ack_rate"])
            time_val.append(key_value["time"])
        pass
    ack_val = np.array(ack_val)
    time_val = np.array(time_val)
    time_val = (time_val - time_val[0])/1000000
    return time_val,ack_val

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

if __name__ == "__main__":
    # test2()
    # test3()
    parser = argparse.ArgumentParser(
        description='Input the log file you want to analysis, default directory is current directory')
    parser.add_argument('--log_filename', dest='log_filename', help='log filename you want to parse')
    parser.add_argument('--log_dir', dest='dir', default=".", help='directory of log file')
    args = parser.parse_args()
    print(args.log_filename)
    print(args.dir)
    log_filename = args.log_filename
    log_dir = args.dir
    file = log_dir + log_filename

    x,y = readFile(file)

    filename = log_filename.split('.')[0]
    plotAndSave(x,y,"delivery rate","time(s)","value(kbit/s)",["delivery rate"],filename+'.png')


