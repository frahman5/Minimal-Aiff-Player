play: play.c
	cc play.c -lm -laiff -o play
	# -lm links in the math library, which LinAiff relies on
	# -laiff links in LibAiff. The documentation says you must do this