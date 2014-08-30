play: play.c streamAudio.c
	gcc play.c streamAudio.c -lm -laiff -lportaudio -o play
	# -lm links in the math library, which LibAiff relies on
	# -lportaudio links in libport audio
	# -laiff links in LibAiff. The documentation says you must do this
playIND: playIND.c streamAudio.c
	gcc playIND.c streamAudio.c -lportaudio -o playIND
	
test: playIND.c test.c endian.h
	gcc playIND.c test.c play.c -laiff -o test