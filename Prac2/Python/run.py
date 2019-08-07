import Prac2
import time

for i in range(5):
	print ("-----------"+str(i)+"-----------");
	Prac2.main()
	time.sleep(30)

print ("++++++++++++++++++++++++++++++++++++++++++++++++++++++")
print ("One Minute to cool down")
time.sleep(60)
for i in range(5):
	print ("++++++++++++++++++++++++++++++++++++++++++++++++++++++")
	Prac2.main()
	time.sleep(30)
