from __future__ import division
import matplotlib                                                                       
matplotlib.use('Agg')                                                                   
import matplotlib.pyplot as plt                                                         
                                                                                        
import random

def plothisto1(hlist):                      
    fig = plt.figure()           
    ax = fig.add_subplot('111')       
    ax.hist(hlist, histtype='bar', log=True)         
    plt.xlabel('Number of updates')                                         
    plt.ylabel('Number of prefixes') 
    plt.title('Here\'s our fancy histogram!')  
    plt.savefig('histogram1.pdf', bbox_inches='tight', pad_inches=0.1)
    plt.close()

def plothisto2(hlist):                      
    fig = plt.figure()           
    ax = fig.add_subplot('111')       
    ax.hist(hlist, histtype='bar', log=True, bins=200)         
    plt.xlabel('Time difference between 2 consecutive updates')   
    plt.ylabel('Number of updates') 
    plt.title('Here\'s our fancy histogram!')  
    plt.savefig('histogram2log.pdf', bbox_inches='tight', pad_inches=0.1)
    plt.close()

def plothisto3(hlist):                      
    fig = plt.figure()           
    ax = fig.add_subplot('111')       
    ax.hist(hlist, histtype='bar', log=False, bins=200)         
    plt.xlabel('Time difference between 2 consecutive updates')   
    plt.ylabel('Number of updates') 
    plt.title('Here\'s our fancy histogram!')  
    plt.savefig('histogram2.pdf', bbox_inches='tight', pad_inches=0.1)
    plt.close()
                
def findPrefixes():
	ribSnapshot = open("rib.20120301.0000.txt") 
	dic = {}
	for line in ribSnapshot:
		temp = line.split("|") #fifth column = prefix
		if (temp[4]=="6939"):
			prefix = temp[5]
			if ("::" not in prefix): #not IPv6
				if (not dic.has_key(prefix)):
					dic[prefix]=[0, 0, 0]
	
	updates = open("updates.20120301.10h.txt")
	notUpdated=len(dic)
	UpdateTimes = []
	i = 0
	for line in updates:
		temp = line.split("|") #fifth column = prefix
		if (temp[4]=="6939"):
			prefix = temp[5] #fifth column = prefix
			if ("::" not in prefix): #not IPv6
				if (prefix in dic):
					if (dic[prefix][0]==0): #keep track of IP prefixes experiencing update only for their first update
						notUpdated-=1
					if (dic[prefix][1]==0): #first update
						dic[prefix][0]=dic[prefix][0]+1
						dic[prefix][1]=temp[1]
					else:
						dic[prefix][0]=dic[prefix][0]+1
						dic[prefix][2]=temp[1] #last update

	print "(a) What fraction of IP prefixes experience no update messages?"
	print notUpdated/len(dic)

	print "(b) What prefix experiences the most updates, and how frequent are they (in updates per minute)?"
	maxx = 0
	maxxprefix = ""
	for prefix in dic: #find prefix with maximum number of updates
		if dic[prefix][0] > maxx:
			maxx = dic[prefix][0]
			maxxprefix = prefix

	#extract value from timestamp string:
	startTime = dic[maxxprefix][1].split()[1].split(":")
	startMinutes = int(startTime[0])*60 + int(startTime[1]) + int(startTime[2])/60
	endTime = dic[maxxprefix][2].split()[1].split(":")
	endMinutes = int(endTime[0])*60 + int(endTime[1]) + int(endTime[2])/60
	totalTime = endMinutes-startMinutes
	print prefix 
	print "num updates: "
	print maxx 
	print "updates/minute: "
	print maxx//totalTime

	print "(c) Create a plot of the number of updates per prefix."
	numUpdates = []
	for prefix in dic:
		numUpdates.append(dic[prefix][0])
	plothisto1(numUpdates)
	return dic

def findEvents(dic):
	updates = open("updates.20120301.10h.txt")
	for line in updates:
		temp = line.split("|") #fifth column = prefix
		if (temp[4]=="6939"):
			prefix = temp[5] #fifth column = prefix
			if ("::" not in prefix and prefix in dic): #not IPv6 but in update file
				time = temp[1].split()[1].split(":")
				seconds = int(time[0])*3600 + int(time[1])*60 + int(time[2])
				if (dic[prefix]): #if this is not the first update
					dic[prefix].append(seconds-dic[prefix][0])
					dic[prefix][0]=seconds #update first value as most recent update time
				else: #first update seen, so start list
					dic[prefix]=[seconds]
	timeIntervals = []
	for prefix in dic:
		if (dic[prefix]): #only include prefixes that have updates
			for x in range(1, len(dic[prefix])):
				timeIntervals.append(dic[prefix][x])
	#print timeIntervals
	plothisto2(timeIntervals)
	plothisto3(timeIntervals)

#main method:
dic = findPrefixes()
newDic = dic.fromkeys(dic)
findEvents(newDic)



