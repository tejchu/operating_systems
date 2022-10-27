import matplotlib.pyplot as plt
  
#times have been aquired using the system call time - discussion at the end

x1 = [2,4,6,8,10,12,14,16,18,20,22, 30, 36, 44, 52, 58, 64] #threads
y1 = [0.99,1.08,1.24,1.34,1.42,1.44,1.42,1.40,1.41,1.47,1.45,1.49,1.52, 1.43,1.52,1.53,1.51] #user
  
# plotting user space time
plt.plot(x1, y1,label = "user space")

x2 = [2,4,6,8,10,12,14,16,18,20,22,30, 36, 44, 52, 58, 64] #threads
y2 = [0.829,0.365,0.299,0.267,0.260,0.258,0.243,0.244,0.234,0.235,0.234,0.227,0.215, 0.228,0.225,0.216,0.224] #total

# plotting real time
plt.plot(x2, y2,label = "real time")

x3 = [2,4,6,8,10,12,14,16,18,20,22,30, 36, 44, 52, 58, 64] #threads
y3 = [0.120,0.296,0.415,0.503,0.621,0.559,0.587,0.577,0.604,0.629,0.622,0.659,0.710,0.631, 0.680, 0.712,0.674] #cpu 

# plotting cpu portion used
plt.plot(x3, y3,label = "cpu portion used")

x4 = [2,4,6,8,10,12,14,16,18,20,22,30, 36, 44, 52, 58, 64] #threads
y4 = [0.00,0.00,0.00,0.00,0.01,0.01,0.01,0.01,0.01,0.01,0.01, 0.01,0.01, 0.01,0.01,0.01,0.01] #system 

# plotting system time
plt.plot(x4, y4,label = "system time")


# axis labels
plt.xlabel('number of threads')
plt.ylabel('time/portion')
  
#title
plt.title('Execution time depending on number of threads')
#showing plot with legend
plt.legend()
plt.show()

