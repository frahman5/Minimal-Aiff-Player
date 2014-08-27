play: play.c
	gcc play.c -lm -laiff -lportaudio -o play
	# -lm links in the math library, which LibAiff relies on
	# -l "libportaudio.a" links in lib port audio
	# -laiff links in LibAiff. The documentation says you must do this
test: play.c test.c
	gcc play.c test.c -lm -lportaudio -laiff  -o test