all: the-client the-server x11app

pkg/generated:
	mkdir -p pkg/generated
	go mod tidy

pkg/generated/mandelbrot.pb.go: pkg/generated protobuf-src/mandelbrot.proto
	protoc -I=protobuf-src/ --go_out=plugins=grpc:pkg/generated protobuf-src/*.proto

cpp/generated/mandelbrot.pb.cc: protobuf-src/mandelbrot.proto
	protoc -I=protobuf-src/ --cpp_out=cpp/generated protobuf-src/*.proto

the-client: go-client/main.go pkg/generated/mandelbrot.pb.go
	go build -o the-client ./go-client/

the-server: go-server/main.go pkg/generated/mandelbrot.pb.go 
	go build -o the-server ./go-server

x11app: cppXapp/theprog.cpp cpp/generated/mandelbrot.pb.cc
	g++ -o x11app -I /include ./cppXapp/theprog.cpp -lstdc++ -lX11 -lm

clean: 
	rm -rf pkg/generated/*
	rm -rf pkg/health/*
	rm -rf cpp/generated/*
	rm -f the-server
	rm -f the-client
	rm -f x11app
