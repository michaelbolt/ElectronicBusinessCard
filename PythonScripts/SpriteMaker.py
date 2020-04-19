try:
    from tkinter import *  # Python 3.x
    from tkinter import ttk
except ImportError:
    from Tkinter import *  # Python 2.x
    import ttk
import threading
import time
from argparse import ArgumentParser

class spriteMaker :

	def __init__(self, pixelSize = 15, spriteWidth = 8):
		"""spriteMaker Constructor
		Creates root window for GUI
		"""
		self.root = Tk()
		self.root.title('SSD1306 Sprite Maker v1.0')
		self.pixelSize = pixelSize
		self.spriteWidth = spriteWidth
		self.canvasWidth = pixelSize * spriteWidth
		self.canvasHeight = pixelSize * 8
		self.animating = False
		self.placeWidgets()


	def startApplication(self):
		"""Starts SSD1306 Sprite Maker application
		"""
		self.root.mainloop()


	def placeWidgets(self):
		"""Places widgets on screen
		"""
		#### create canvas for sprite
		self.pixelCanvasFrame = ttk.Frame(self.root)
		self.pixelCanvasFrame['padding'] = (10,10)
		self.pixelCanvasFrame.grid(row=0, column=0, rowspan=1, sticky=(N,S,E,W))
		self.pixelCanvas = Canvas(self.pixelCanvasFrame, width=self.canvasWidth+1, 
			height=self.canvasHeight+1, bg="#000000")
		self.pixelCanvas.grid(row=1, column=1)
		# create array of rectangles on canvas
		self.pixels = [[[0 for d2 in range(self.spriteWidth)] for d1 in range(8)] for d0 in range(16)]
		self.pixelRects = [[0 for d1 in range(self.spriteWidth)] for d0 in range(8)]
		for row in range(8):
			for col in range(self.spriteWidth):
				self.pixelRects[row][col] = self.pixelCanvas.create_rectangle(
					3 + col*self.pixelSize, 3 + row*self.pixelSize, 
					3+(col+1)*self.pixelSize, 3+(row+1)*self.pixelSize, 
					outline="#808080", fill="#000000")
		# bind mouse click within canvas widget
		self.pixelCanvas.bind("<Button-1>",self.canvasClick)
		# label for rame number
		ttk.Label(self.pixelCanvasFrame, text=' ', anchor=S).grid(row=0, column=0, columnspan=3, sticky=(N,S,E,W))
		self.currentFrameLabel = ttk.Label(self.pixelCanvasFrame, text='Frame 1', anchor=N, width=10)
		self.currentFrameLabel.grid(row=2, column=0, columnspan=3, sticky=(N,S,E,W))
		self.pixelCanvasFrame.rowconfigure(0, weight=1)
		self.pixelCanvasFrame.rowconfigure(2, weight=1)
		self.pixelCanvasFrame.columnconfigure(0, weight=1)
		self.pixelCanvasFrame.columnconfigure(2, weight=1)

		#### code output text box
		self.codeText = 0	
		self.codeFrame = ttk.Frame(self.root)
		self.codeFrame['padding'] = (10,10)
		self.codeFrame.grid(row=1, column=0, columnspan=3, sticky=(N,S,E,W))
		self.code = Text(self.codeFrame, width = max(8+self.spriteWidth*6, 52), height= 18)
		self.code.configure(font='menlo 12')
		self.code.grid(row=0, column=0, sticky=(N,S,E,W))

		### sprite options
		self.optionsFrame = ttk.Frame(self.root)
		self.optionsFrame['padding'] = (10,10)
		self.optionsFrame.grid(row=0, column=1, sticky=(N,S,E,W))
		# sprite name
		ttk.Label(self.optionsFrame, text='Sprite name:', anchor=E).grid(row=1, column=0, sticky=(E,W))
		vcmd = (self.optionsFrame.register(self.spriteNameChange), '%P')
		self.spriteName = 'sprite'
		self.spriteNameEntry = ttk.Entry(self.optionsFrame, width=10)
		self.spriteNameEntry.insert(0,'sprite')
		self.spriteNameEntry['validate']='key'
		self.spriteNameEntry['validatecommand']=vcmd
		self.spriteNameEntry.grid(row=1, column=1, columnspan=2, sticky=(E,W))
		# current frame
		ttk.Label(self.optionsFrame, text='Current frame:', anchor=E).grid(row=3, column=0, sticky=(E,W))
		vcmd = (self.optionsFrame.register(self.currentFrameChange), '%P')
		self.currentFrame = 0
		self.currentFrameSpinbox = Spinbox(self.optionsFrame, from_=1, to_=1, width=10)
		self.currentFrameSpinbox.insert(0,'1')
		self.currentFrameSpinbox['validate']='key'
		self.currentFrameSpinbox['validatecommand']=vcmd
		self.currentFrameSpinbox.grid(row=3, column=1, columnspan=2, sticky=(E,W))
		# frame count
		ttk.Label(self.optionsFrame, text='Frame count:', anchor=E).grid(row=2, column=0, sticky=(E,W))
		vcmd = (self.optionsFrame.register(self.frameCountChange), '%P')
		self.frameCount = 1
		self.frameCountSpinbox = Spinbox(self.optionsFrame, from_=1, to_=16, 
			validate='key', validatecommand=vcmd, width=10)
		self.frameCountSpinbox.grid(row=2, column=1, columnspan=2, sticky=(E,W))
		# frames per second & animate
		ttk.Label(self.optionsFrame, text='FPS:', anchor=E).grid(row=4, column=0, sticky=(E,W))
		self.fps = 1
		self.fpsSpinbox = Spinbox(self.optionsFrame, from_=1, to_=32, width=3)
		self.fpsSpinbox.delete(0,'end')
		self.fpsSpinbox.insert(0,'16')
		self.fpsSpinbox.grid(row=4, column=1, sticky=(E,W))
		self.animateBtn = ttk.Button(self.optionsFrame, text='Animate', command=self.animateBtnPres)
		self.animateBtn.grid(row=4, column=2, sticky=(E,W))
		# empty labels for spacing
		ttk.Label(self.optionsFrame, text=' ', width=1).grid(row=0, column=0, columnspan=3, sticky=(N,S,E,W))
		ttk.Label(self.optionsFrame, text=' ', width=1).grid(row=5, column=0, columnspan=3, sticky=(N,S,E,W))
		self.optionsFrame.rowconfigure(0, weight=1)
		self.optionsFrame.rowconfigure(5, weight=1)
		self.optionsFrame.columnconfigure(0, weight=1)

		# configure row/column stretch	
		self.root.rowconfigure(0, weight=1)
		self.root.columnconfigure(0, weight=1)
		self.root.columnconfigure(1, weight=1)


	def canvasClick(self,event):
		"""Handle click event updates within pixelCanvas
		"""
		try:
			id = self.pixelCanvas.find_withtag(CURRENT)
			(x, y, x2, y2) = self.pixelCanvas.bbox(CURRENT)
			# update pixel data
			row = min(y // self.pixelSize, 7)
			col = min(x // self.pixelSize, self.spriteWidth-1)
			self.pixels[self.currentFrame][row][col] ^= 0x01
			# update selected rectangle color
			if self.pixels[self.currentFrame][row][col]:
				self.pixelCanvas.itemconfig(self.pixelRects[row][col], fill="#FFFFFF")
			else:
				self.pixelCanvas.itemconfig(self.pixelRects[row][col], fill="#000000")
			# update code output
			self.writeFrameCode(self.currentFrame)
		# catch out-of-bounds click
		except TypeError:
			pass


	def writeFrameCode(self, frameNumber):
		"""write a single line of pixel code in the code window
		"""
		self.code.delete(str(frameNumber+2)+'.0', str(frameNumber+2)+'.end')
		self.code.insert(str(frameNumber+2)+'.0', '    ')
		if self.frameCount > 1:
			self.code.insert(str(frameNumber+2)+'.end', '{')
		for col in range(self.spriteWidth):
			value = 0
			for row in range(8):
				value *= 2
				if self.pixels[frameNumber][row][col]:
					value += 1
			self.code.insert(str(frameNumber+2)+'.end','0x{:02X}'.format(value))
			if col is not self.spriteWidth-1:
				self.code.insert(str(frameNumber+2)+'.end',', ')
			elif self.frameCount > 1:
				self.code.insert(str(frameNumber+2)+'.end','}')
				if frameNumber != self.frameCount-1:
					self.code.insert(str(frameNumber+2)+'.end',',')



	def spriteNameChange(self, P):
		"""Update sprite name in code output
		"""
		if P:
			self.spriteName = P
			self.code.delete('1.0','end')
			if self.frameCount is 1:
				self.code.insert('1.0','const uint8_t ' + self.spriteName + '[' + str(self.spriteWidth) + '] = {\n')
			else:
				self.code.insert('1.0','const uint8_t ' + self.spriteName + '[' + P + '][' + str(self.spriteWidth) + '] = {\n')
		return True


	def frameCountChange(self, P):
		"""Update frame count and rewrite code to reflect array size
		"""
		if P:
			self.frameCount = int(P)
			# update currentFrameSpinbox
			self.currentFrameSpinbox['to_'] = self.frameCount
			# rewrite code with new number of frame	
			self.code.delete('1.0','end')
			if self.frameCount is 1:
				self.code.insert('1.0','const uint8_t ' + self.spriteName + '[' + str(self.spriteWidth) + '] = {\n')
			else:
				self.code.insert('1.0','const uint8_t ' + self.spriteName + '[' + P + '][' + str(self.spriteWidth) + '] = {\n')
			for frame in range(self.frameCount):
				self.writeFrameCode(frame)
				self.code.insert(str(frame+2)+'.end','\n')
			self.code.insert('end','    };')
			# if the selected frame was higher than new frame count, adjust
			if self.currentFrame >= self.frameCount:
				self.currentFrameSpinbox.delete(0,'end')
				self.currentFrameSpinbox.insert(0,str(self.frameCount))
		return True


	def currentFrameChange(self, P):
		"""Change current frame
		"""
		if P:
			# update current frame id
			self.currentFrame = int(P) - 1
			self.currentFrameLabel['text'] = 'Frame ' + str(self.currentFrame+1)
			# draw newly selected frame
			for row in range(8):
				for col in range(self.spriteWidth):
					if self.pixels[self.currentFrame][row][col]:
						self.pixelCanvas.itemconfig(self.pixelRects[row][col], fill="#FFFFFF")
					else:
						self.pixelCanvas.itemconfig(self.pixelRects[row][col], fill="#000000")
		return True


	def animateBtnPres(self):
		# if animation is running, stop
		if self.animating:
			self.animating = False
			self.animateBtn['text'] = 'Animate'
		# if no animation is running, start
		else:
			self.animating = True
			self.animateBtn['text'] = 'Stop'
			self.animateCounter = 0
			# set up periodic interrupt according to fps
			self.next_t = time.time()
			self.animationThread()


	def animationThread(self):
		# thread is in Try-Except in case the window was closed since the thread was started
		try:
			# if 'Stop' hasn't been pressed
			if self.animating:
				# draw frame
				self.currentFrameLabel['text'] = 'Frame ' + str(self.animateCounter+1)
				for row in range(8):
					for col in range(self.spriteWidth):
						if self.pixels[self.animateCounter][row][col]:
							self.pixelCanvas.itemconfig(self.pixelRects[row][col], fill="#FFFFFF")
						else:
							self.pixelCanvas.itemconfig(self.pixelRects[row][col], fill="#000000")
				# increment and rollover frame for drawing
				self.animateCounter += 1
				if self.animateCounter >= self.frameCount:
					self.animateCounter = 0
				# prepare next interrupt if 'Stop' hasn't been pressed
				self.fps = float(self.fpsSpinbox.get())
				self.next_t += 1 / float(self.fps)
				threading.Timer(self.next_t - time.time(), self.animationThread).start()
			# else, redraw current frame
			else:
				for row in range(8):
					for col in range(self.spriteWidth):
						if self.pixels[self.currentFrame][row][col]:
							self.pixelCanvas.itemconfig(self.pixelRects[row][col], fill="#FFFFFF")
						else:
							self.pixelCanvas.itemconfig(self.pixelRects[row][col], fill="#000000")
		except:
			pass



##### main program
def main():
	parser = ArgumentParser(description='GUI to generate sprite data for SSD1306.')
	parser.add_argument('-w','--width', dest='spriteWidth', default=8,
		help='width of sprite in pixels')
	parser.add_argument('-d','--display', dest='pixelSize', default=15,
		help='sprite pixel display size')
	args = parser.parse_args()
	App = spriteMaker(spriteWidth=int(args.spriteWidth), pixelSize=int(args.pixelSize))
	App.startApplication()


if __name__ == "__main__":
    main()




