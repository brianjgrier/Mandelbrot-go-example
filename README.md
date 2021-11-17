# Mandelbrot Golang and C/C++ example

This code is an example of how to use protocol buffers in multiple languages. I am targeting Golang for the server and Golang, C/C++ and C# (future work) for the client languages. 

The code is writen to be tested via the command line with the go-client application. The go-server includes a Dockerfile to build a container. Publishing to Docker Hub will be done via CircleCI.

You will need to set the environment variable PROTOINCLUDE to point to where you have installed the C++ protobuf libraries.

---

# Installing the PROTOBUF compiler and C++ libraries

Follow the instructions at the following URL

> https://grpc.io/docs/languages/cpp/quickstart/

If you are having problems compiling that package there are a couple of changes to their instructions you can make:

> git clone --recurse-submodules -b <branch_id> https://github.com/grpc/grpc

should be changed to:

> git clone --recurse-submodules https://github.com/grpc/grpc


In the build command section if you are on a small system (less than 16GB):

> make -j 

should be

> make -j 3

---

If you already have a PROTOCOL Buffer compiler installed you ** MUST ** use either this compiler or one with the same version. 

---

## Background information
A copy of the original 1985 article that sparked my interest in Fractals can be found in the PDF file in the root of this repository. This is a photocopy of the original article so you have to search around the ads for the meat of the infromation. I pulled this copy from: 

https://www.scientificamerican.com/article/mandelbrot-set-1990-horgan/

I originally implemented this in Basic on an Atari-400 computer and it required over 8 hours to render a 640x480 image.