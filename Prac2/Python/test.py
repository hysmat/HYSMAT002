f = open("../data/accData.csv", "r")
data = f.read()
f.close()
for i in range(5):
	print (data[i])

