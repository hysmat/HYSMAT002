#!/usr/bin/python3
"""
Python Practical 2 Code for heterodyning and performance testing
Keegan Crankshaw
EEE3096S Prac 2 2019
Date: 7 June 2019

This isn't exactly performant code, but it is Pythonic
This is done to stress the differences between Python and C/C++

"""

# import Relevant Librares
import Timing
from data import carrier, data

# Define values.
c = carrier *30
d = data *30
result = []

def saveData(arr):
    outData = open("../data/accData.csv", "w+");
    for i in arr:
        outData.write("{},".format(i))
    outData.close()


# Main function
def main():
    print("using type {}".format(type(data[0])))
    #f = open("../data/python.csv", 'a')
    Timing.startlog()
    for i in range(len(c)):
        result.append(c[i] * d[i])
    elapsed = Timing.endlog()
    #f.write(str(elapsed))
    #f.write("\n")
    #f.close()
    saveData(result)

# Only run the functions if this module is run
if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("Exiting gracefully")
    except Exception as e:
        print("Error: {}".format(e))

