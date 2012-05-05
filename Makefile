main: r00922006_vlc_dec my_encode my_decode

r00922006_vlc_dec: decode.cc
	g++ -o r00922006_vlc_dec.exe decode.cc

my_encode: my_encode.cc
	g++ -o encoder my_encode.cc

my_decode: my_decode.cc
	g++ -o decoder my_decode.cc
