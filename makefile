play: play.c
	cc play.c -lm -laiff -lportaudio -o play
	# -lm links in the math library, which LinAiff relies on
	# -l "libportaudio.a" links in lib port audio
	# -laiff links in LibAiff. The documentation says you must do this