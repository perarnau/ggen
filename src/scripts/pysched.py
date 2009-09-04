#!/usr/bin/env python
# -*- coding: utf-8 -*-


# A Particularly Stupid Scheduling simulator
# Copyright Swann Perarnau 2009
# Licenced under the terms of the GPLv3.
# contact: firstname . lastname AT imag.fr



import getopt,sys
import pygraphviz as pgv

#######################################
# Global options
#######################################
verbose = False

#######################################
# Scheduler
#######################################
class Scheduler:
	""" Implements a stupid algorithm for scheduling """
	def __init__(self,graph):
		self.data = {}
		for n in graph.nodes():
			self.data[n] = graph.out_degree(n)
			if verbose: print "Sched: Node "+n+ " has been given value "+repr(self.data[n])


	def sched(self,tasks_ready):
			
		def task_key(a):
			return self.data[a]

		return max(tasks_ready,key=task_key)


#######################################
# Simulator
#######################################

def simulation(graph,sched,ncpus = 10):
	
	# We need: - a hash for each CPU containing the next idle slot
	#          - a list of ready tasks and one of the other tasks
	#          - a hash with a status for each task (0 ready, 1 executing, 2 unavailable, 3 done)
	#          - a time counter
	cpu_status = {}
	for i in range(ncpus):
		cpu_status[i] = (0,None)

	# iterate over the nodes to find sources
	ready = []
	executing = []
	others = []
	task_status = {}
	for n in graph.nodes():
		if graph.in_degree(n) == 0:
			ready.append(n)
			task_status[n] = 0
		else:
			others.append(n)
			task_status[n] = 2

	# init the time counter
	time = 0

	print "Launching simulation"
	# loop until all the tasks are executed
	while len(ready) != 0 or len(others) != 0 or len(executing) != 0:
		if verbose: print "Time is "+repr(time)
		# is it the time to idle one cpu ?
		for i in range(ncpus):
			if cpu_status[i][0] == time:
				if cpu_status[i][1] != None:
					finished_task = cpu_status[i][1]
					task_status[finished_task] = 3
					executing.remove(finished_task)
					cpu_status[i] = (0,None)
					if verbose: print "Task "+finished_task+ " terminated"
		
		# update others and ready lists
		# warning : must iterate over a copy of the list !!
		for n in others[:]:
			if verbose: print "Studying readiness of "+n
			for father in graph.in_neighbors(n):
				if verbose: print "Predecessor "+father+ " has status "+repr(task_status[father])
				if task_status[father] != 3:
					break
			else:
				if verbose: print "Task "+n+" is ready"
				task_status[n] = 0
				others.remove(n)
				ready.append(n)
		
		# print ready
		if verbose: print "Task ready are: "+ repr(ready)

		# now iterate over the list of available processors
		for i in range(ncpus):
			if cpu_status[i][0] == 0 and len(ready) != 0:
				# find a task to execute
				task = sched.sched(ready)
				if verbose: print "Choosed cpu "+repr(i)+ " and task "+task
				# update cpu and ready list status
				cpu_status[i] = (time+ 1, task)
				task_status[task] = 1
				ready.remove(task)
				executing.append(task)
		# next time
		time += 1
	
	# time is off by one
	time -= 1

	print "Simulation done"
	print "Completion time: "+repr(time)

#######################################
# Main functions
#######################################
def usage():
	print """
Usage: pysched [options]
Options:
	-i,--input <filename>	use filename as input.
	-v			be verbose.
	-h,--help		print this help.
"""

def main():
	global verbose
	try:
		opts, args = getopt.getopt(sys.argv[1:], "hi:v", ["help", "input="])
	except getopt.GetoptError, err:
		# print help information and exit:
		print str(err) # will print something like "option -a not recognized"
    		usage()
		sys.exit(2)
	input = None
	for o, a in opts:
		if o == "-v":
			verbose = True
		elif o in ("-h", "--help"):
			usage()
			sys.exit()
		elif o in ("-i", "--input"):
			input = a
		else:
			assert False, "unhandled option"

	print "Verbose is "+repr(verbose)
	# read Graph
	# we are handling DAGs here !
	Graph = pgv.AGraph(input)
	
	S = Scheduler(Graph)

	# simulate
	simulation(Graph,sched = S)

if __name__ == "__main__":
	main()
