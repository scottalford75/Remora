#!/usr/bin/env python2
"""Copyright (C) 2020 Scott Alford, scottalford75@gmail.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU 2 General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
"""

update = 0.05	# this is how often the z external offset value is updated based on current x & y position 

import sys
import os.path, time
import numpy as np
from scipy.interpolate import griddata
from enum import Enum, unique

import linuxcnc

@unique
class States(Enum):
    START = 1
    IDLE = 2
    LOADMAP = 3
    RUNNING = 4
    RESET = 5
    STOP = 6


class Compensation :
	def __init__(self) :
		self.comp = {}
		if len(sys.argv)<2:
			print "ERROR! No input file name specified!"
			sys.exit()

		self.filename = sys.argv[1]
		self.method = sys.argv[2]
		
		# default to cubic if not specified
		if self.method == "" : self.methond = "cubic"


	def loadMap(self) :
		# data coordinates and values
		self.data = np.loadtxt(self.filename, dtype=float, delimiter=" ", usecols=(0, 1, 2))
		self.x_data = np.around(self.data[:,0],1)
		self.y_data = np.around(self.data[:,1],1)
		self.z_data = self.data[:,2]

		# get the x and y, min and max values from the data
		self.xMin = int(np.min(self.x_data))
		self.xMax = int(np.max(self.x_data))
		self.yMin = int(np.min(self.y_data))
		self.yMax = int(np.max(self.y_data))

		print "	xMin = ", self.xMin
		print "	xMax = ", self.xMax
		print "	yMin = ", self.yMin
		print "	yMax = ", self.yMax

		# target grid to interpolate to, 1 grid per mm
		self.xSteps = (self.xMax-self.yMin)+1
		self.ySteps = (self.yMax-self.yMin)+1
		self.x = np.linspace(self.xMin, self.xMax, self.xSteps)
		self.y = np.linspace(self.yMin, self.yMax, self.ySteps)
		self.xi,self.yi = np.meshgrid(self.x,self.y)
		
		# interpolate, zi has all the offset values but need to be transposed
		self.zi = griddata((self.x_data,self.y_data),self.z_data,(self.xi,self.yi),method=self.method)
		self.zi = np.transpose(self.zi)

		
	def compensate(self) :
		# get our nearest integer position
		self.xpos = int(round(self.h['x-pos']))
		self.ypos = int(round(self.h['y-pos']))
		
		# clamp the range
		self.xpos = self.xMin if self.xpos < self.xMin else self.xMax if self.xpos > self.xMax else self.xpos
		self.ypos = self.yMin if self.ypos < self.yMin else self.yMax if self.ypos > self.yMax else self.ypos
		
		# location in the offset map array
		self.Xn = self.xpos - self.xMin
		self.Yn = self.ypos - self.yMin
		
		# get the nearest compensation offset and convert to counts (s32) with a scale (float) 
		# Requested offset == counts * scale
		self.scale = 0.001
		zo = self.zi[self.Xn,self.Yn]
		compensation = int(zo / self.scale)
		
		return compensation


	def run(self) :
		import hal, time
		
		self.h = hal.component("compensation")
		self.h.newpin("enable-in", hal.HAL_BIT, hal.HAL_IN)
		self.h.newpin("enable-out", hal.HAL_BIT, hal.HAL_OUT)
		self.h.newpin("scale", hal.HAL_FLOAT, hal.HAL_IN)
		self.h.newpin("counts", hal.HAL_S32, hal.HAL_OUT)
		self.h.newpin("clear", hal.HAL_BIT, hal.HAL_IN)
		self.h.newpin("x-pos", hal.HAL_FLOAT, hal.HAL_IN)
		self.h.newpin("y-pos", hal.HAL_FLOAT, hal.HAL_IN)
		self.h.newpin("z-pos", hal.HAL_FLOAT, hal.HAL_IN)
		self.h.newpin("fade-height", hal.HAL_FLOAT, hal.HAL_IN)
		self.h.ready()
		
		s = linuxcnc.stat()
		
		currentState = States.START
		prevState = States.STOP

		try:
			while True:
				time.sleep(update)
				
				# get linuxcnc task_state status for machine on / off transitions
				s.poll()
				
				if currentState == States.START :
					if currentState != prevState :
						print("\nCompensation entering START state")
						prevState = currentState
						
					# do start-up tasks
					print(" %s last modified: %s" % (self.filename, time.ctime(os.path.getmtime(self.filename))))
					
					prevMapTime = 0
					
					self.h["counts"] = 0
					
					# transition to IDLE state
					currentState = States.IDLE
				
				elif currentState == States.IDLE :
					if currentState != prevState :
						print("\nCompensation entering IDLE state")
						prevState = currentState
						
					# stay in IDLE state until compensation is enabled
					if self.h["enable-in"] :
						currentState = States.LOADMAP
			
				elif currentState == States.LOADMAP :
					if currentState != prevState :
						print("\nCompensation entering LOADMAP state")
						prevState = currentState
			
					mapTime = os.path.getmtime(self.filename)
			
					if mapTime != prevMapTime:
						self.loadMap()
						print("	Compensation map loaded")
						prevMapTime = mapTime
					
					# transition to RUNNING state
					currentState = States.RUNNING
				
				elif currentState == States.RUNNING :
					if currentState != prevState :
						print("\nCompensation entering RUNNING state")
						prevState = currentState
			
					if self.h["enable-in"] :
						# enable external offsets
						self.h["enable-out"] = 1
						
						fadeHeight = self.h["fade-height"]
						zPos = self.h["z-pos"]
						
						if fadeHeight == 0 :
							compScale = 1
						elif zPos < fadeHeight :
							compScale = (fadeHeight - zPos)/fadeHeight
							if compScale > 1 :
								compScale = 1
						else :
							compScale = 0
							
						if s.task_state == linuxcnc.STATE_ON :
							# get the compensation if machine power is on, else set to 0
							# otherwise we loose compensation eoffset if machine power is cycled 
							# when copensation is enable
							compensation = self.compensate()
							self.h["counts"] = compensation * compScale
							self.h["scale"] = self.scale
						else :
							self.h["counts"] = 0
						
					else :
						# transition to RESET state
						currentState = States.RESET
						
				elif currentState == States.RESET :
					if currentState != prevState :
						print("\nCompensation entering RESET state")
						prevState = currentState

					# reset the eoffsets counts register so we don't accumulate
					self.h["counts"] = 0

					# toggle the clear output
					self.h["clear"] = 1;
					time.sleep(0.1)
					self.h["clear"] = 0;

					# disable external offsets
					self.h["enable-out"] = 0
					
					# transition to IDLE state
					currentState = States.IDLE

		except KeyboardInterrupt:
	  	  raise SystemExit

comp = Compensation()
comp.run()
