Multithreaded Spellcheck Server

A. Description

Spellcheck is a program using multithreading and server-client connection to check the inputs' spelling from the users. The program includes a default library file (.txt);however, this file can be replaced for better spellcheck.

B. Execute the program

	1. First way to test the program, open the program with one computer, then connect with other computers to test the spellchecking.

	2. The second way to test this program is using only a computer. The testers will copy everything to the main folder, then open the spellchecker in background mode (with command "./spellchecker &"). Then type "telnet 127.0.0.1 11111" while still in your terminal, to self-connect to the server (127.0.0.1 is the loopback address, 11111 is the default port number stored in the file). This method cannote test the multithread functionality of the program. However, it's faster to check the spellchecking functionality. 
C. Functionality

The program will return the word inputted by the users, and there are two situations:
	1. The program will return "OK" if the input is in the given dictionary file 
	2  The program will return "MISPELLED" if the dictionary file doesn't inlcude the inputted word.


