main: r00922006_vlc_dec my_encode my_decode
# 	./my_encode input.txt my.bin
# 	./my_decode my.bin my.txt
	./my_encode world95.txt output.bin && ./my_decode output.bin decode.txt
# 	./r00922006_vlc_dec.exe K5 compressed.txt decoded_symbol.txt

r00922006_vlc_dec: decode.cc
	g++ -o r00922006_vlc_dec.exe decode.cc

my_encode: my_encode.cc
	g++ -o my_encode my_encode.cc

my_decode: my_decode.cc
	g++ -o my_decode my_decode.cc
