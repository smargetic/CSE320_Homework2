# CSE320_Homework2

#This assignment is a debugging assignment, so the majority of the code was not written by me.
#Selective areas were written by me, in order the make the code operational.
#In order to be operational, the code needed several memory leaks to be fixed, as well as other various bugs.

#The code attached is completely operational, and recieved a perfect score.

#The code works as a client rolodex, taking information on clients, so as to be searched through later.

#The original argument passing is replaced and the getopt() function is utilized (done in rolo_main()).
#The possible flags that can be passed are, "-l -s -u."
#-l lists the names, work numbers, and home numbers of all current entries.
#-s searches for a particular client name and displays all information on such.
#-u must be immediately followed by a user name. A pointer to the user name will be saved as a global variable.
#The format of the command line arguments are "rolo [person1 person2 ...] [-l -s -u user]."
