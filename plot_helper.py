


import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import sys





Input=sys.argv[1]
df=pd.read_csv(Input,names=['Y'])


temp=np.array(df['Y'])


def plot_fcn(y):
    x=['1','2','4']
    y1=[]
    y2=[]
    for i in range(0,3):
    	y1.append(y[i])
    for i in range(3,6):
    	y2.append(y[i])
    plt.figure(figsize=(12,12), dpi=100)
    plt.plot(x,y1, label="code runs on 1 Node",color = "blue",marker='*',linestyle='dashed',markerfacecolor='red')
    plt.plot(x,y2, label="code runs on 2 Node",color = "red",marker='*',linestyle='dashed',markerfacecolor='red')
    plt.xlabel("variation in ppn")
    plt.ylabel("Time in sec")
    plt.title("Time vs ppn")
    plt.legend()
    plt.savefig("plot.jpg")

plot_fcn(temp)
