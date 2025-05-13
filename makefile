
phony: run

run: main.cpp
	g++ -std=gnu++20 -o main \
 main.cpp \
 /root/softposit/soft-posit-cpp/build/libsoftposit.a  \
 -I/root/softposit/soft-posit-cpp/include  \
 -I/root/eigen-3.4.0 \
 -O3 && ./main 