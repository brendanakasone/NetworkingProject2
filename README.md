# CNT 4007 Project 2

# Description
Project 2 is a file synchronization application. Multiple clients can concurrently synchronize their local files with a common, shared server. A server and client was developed to provide this functionality. 

# Setting Up
Compile the code using:
```
make
```

Running the Server:
```
./server <directory name>
```

Running the Client:
```
./client <directory name>
```

# How to Use
All options are case sensitive:  
List Files: list all files available on the server    
Diff: checks for differing server and client files  
Pull: obstains files found in diff locally   
Leave: closes the current client  

# Description
Project 2 is a file synchronization application. Multiple clients can concurrently synchronize their local files with a common, shared server. A server and client was developed to provide this functionality. 

# Setting Up
SSH into the UF Server  
Compile the code using:
```
make
```

Running the Server:
```
./server <directory name>
```

Running the Client:
```
./client <directory name>
```

# Bugs/Errors
1. User Options are case sensitive
2. Pull functionality not working
3. The history.txt file not present