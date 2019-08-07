# Notes


I added a loop that checks if the output data is correct and then adds the number of errors next to the time in the csv file (I also added the value of the minimum acceptable error).

The program only runs once but it can be run multiple times using the terminal command:

`
$ for i in `seq <Number of times you want the program to run>` ;do <command e.g python Prac2.py>; done; 
`


Every time the program is run it will append the data to the csv file.
