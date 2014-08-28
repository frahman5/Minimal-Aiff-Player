play: play.c streamAudio.c
	gcc play.c streamAudio.c -lm -laiff -lportaudio -o play
	# -lm links in the math library, which LibAiff relies on
	# -l "libportaudio.a" links in lib port audio
	# -laiff links in LibAiff. The documentation says you must do this
test: playIND.c test.c
	gcc playIND.c test.c -o test