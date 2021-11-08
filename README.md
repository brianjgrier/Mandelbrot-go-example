# Mandelbrot Golang and C/C++ example

This code is an example of how to use protocol buffers in multiple languages. I am targeting Golang for the server and Golang, C/C++ and C# (future work) for the client languages. 

The code is writen to be tested via the command line with the go-client application. The go-server includes a Dockerfile to build a container. Publishing to Docker Hub will be done via CircleCI.

A copy of the original 1985 article that sparked my interest in Fractals can be found in the PDF file in the root of this repository. This is a photocopy of the original article so you have to search around the ads for the meat of the infromation.

I originally implemented this in Basic on an Atari-400 computer and it required over 8 hours to render a 640x480 image.