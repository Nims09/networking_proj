default: default
	gcc -o rdpr rdpr.c
	gcc -o rdps rdps.c
clean:
	rm rdpr
	rm rdps

