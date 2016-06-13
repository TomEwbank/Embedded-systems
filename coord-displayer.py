"""
ldr.py
Display analog data from Arduino using Python (matplotlib)
Author: Mahesh Venkitachalam
Website: electronut.in
"""

import matplotlib.pyplot as plt 
import matplotlib.animation as animation



# call main
if __name__ == '__main__':

  fig = plt.figure()
  ax = fig.add_subplot(1,1,1)
  ax.set_xlim([-200, 200])
  ax.set_ylim([0,300])
  point, = ax.plot([],[], 'or', markersize = 10)
  an = ax.annotate("",xy=(0,0))
  # an = ax.annotate("("+str(x)+", "+str(y)+")",xy=(x+5,y+5))

  def animate(i):
    # data = open('example.txt', 'r').read()
    # x, y = data.split(',')
    x = 50
    y = 100
    point.set_data([x],[y])

    an.set_text("("+str(x)+", "+str(y)+")")
    an.set_x(x+5)
    an.set_y(y+5)
    
    plt.draw()

  ani = animation.FuncAnimation(fig, animate, interval=500)
  plt.show()