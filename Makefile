all : main

main : main.cpp co_routine.cpp
	g++ -o $@ $^ -Wall -g
clean :
	rm -rf main
