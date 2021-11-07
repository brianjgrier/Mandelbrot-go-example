From golang:1.16.3

LABEL maintainer="Brian Grier <brigrier@cisco.com>"

WORKDIR /app

RUN apt-get update
RUN apt install -y protobuf-compiler

RUN GO111MODULE=on \
        go get google.golang.org/protobuf/cmd/protoc-gen-go@v1.26.0 \
        google.golang.org/grpc/cmd/protoc-gen-go-grpc@v1.1.0 \
        github.com/grpc-ecosystem/grpc-gateway/protoc-gen-grpc-gateway \
	    github.com/grpc-ecosystem/grpc-gateway/protoc-gen-swagger \
	    github.com/golang/protobuf/protoc-gen-go

COPY . .

RUN go mod download

RUN protoc -I=protobuf-src/ --go_out=plugins=grpc:pkg/generated protobuf-src/*.proto

RUN go build -a -o mandelbrot-server ./go-server

#
# Using Multi-stage build to get rid of the build and run-time libraries that
# were built into the executable by using the '-a' option

FROM alpine:latest

RUN apk --no-cache add ca-certificates

WORKDIR /app/

COPY --from=0 /app/mandelbrot-server ./

EXPOSE 8083

CMD ["./madelbrot-server"]