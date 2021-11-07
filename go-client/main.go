package main

import (
	"fmt"
	"log"
	"io"

	"context"

	//"github.com/sirupsen/logrus"
	//"google.golang.org/grpc"

	//grpcMandelbrot "github.com/brianjgrier/Mandelbrot/pkg/generated"

	"google.golang.org/grpc"

	grpcMandelbrot "github.com/brianjgrier/Mandelbrot/pkg/generated"
)

type BoundedArea struct {
	upperLeft struct {
		x float64
		y float64
	}
	lowerRight struct {
		x float64
		y float64
	}
}

type WindowArea struct {
	upperLeft struct {
		x int
		y int
	}
	lowerRight struct {
		x int
		y int
	}
}

//func processPoint(x float64, y float64) int {
//	result := -1
//	var x1, y1 float64
//
//	c := complex(x, y)
//	z := complex(0, 0)
//
//	for i := 0; i < 1000; i++ {
//		result = i
//		z = (z * z) + c
//		x1 = real(z)
//		y1 = imag(z)
//
//		if (x1*x1 + y1*y1) > 2 {
//			break
//		}
//
//	}
//
//	return result
//}

func main() {
	var areaOfInterest BoundedArea
	var displayRegion WindowArea
	//var xpos, ypos float64
	//var result int

	areaOfInterest.upperLeft.x = -2.0
	areaOfInterest.upperLeft.y = 1.25
	areaOfInterest.lowerRight.x = 0.5
	areaOfInterest.lowerRight.y = -1.25

	displayRegion.upperLeft.x = 0
	displayRegion.upperLeft.y = 0
	displayRegion.lowerRight.x = 1920
	displayRegion.lowerRight.y = 1080

	//pointValue := 0

	fmt.Println("Starting Mandelbrot client")


	//processOneAtaTime(areaOfInterest, displayRegion, mandelbrotClient)
	processLineAtaTime(areaOfInterest, displayRegion)

	fmt.Println("Done")
}

func processOneAtaTime(fractalRegion BoundedArea, windowRegion WindowArea) {
	var pointValue int = 0

	strideX := (fractalRegion.lowerRight.x - fractalRegion.upperLeft.x) / (float64)(windowRegion.lowerRight.x - windowRegion.upperLeft.x)
	strideY := (fractalRegion.lowerRight.y - fractalRegion.upperLeft.y) / (float64)(windowRegion.lowerRight.y - windowRegion.upperLeft.y)
	fmt.Println("Stride of X:", strideX)
	fmt.Println("Stride of Y:", strideY)

	for y := 0; y <= (windowRegion.lowerRight.y - windowRegion.upperLeft.y); y++ {
		fmt.Println("Proceesing line:", y)
		ypos := fractalRegion.upperLeft.y + ((float64)(y) * strideY)
		for x := 0; x <= (windowRegion.lowerRight.x - windowRegion.upperLeft.x); x++ {
			xpos := fractalRegion.upperLeft.x + ((float64)(x) * strideX)
			//result = processPoint(xpos, ypos)

			grpcPoint := &grpcMandelbrot.ComplexPoint{
				Real:      xpos,
				Imaginary: ypos,
			}

			grpcReq := &grpcMandelbrot.PointRequest{
				ThePoint: grpcPoint,
			}

			cc, err := grpc.Dial("127.0.0.1:50051", grpc.WithInsecure())
			if err != nil {
				log.Fatalln("Error establishing connection:", err)
			}
		
			mandelbrotClient := grpcMandelbrot.NewComplexPointProcessingServiceClient(cc)
		
			result, err := mandelbrotClient.ProcessPoint(context.Background(), grpcReq)
			if err != nil {
				log.Fatalln("Error while calling Process Point:", err)
			}
			pointValue = (int)(result.GetResult())
			cc.Close()
		}
	}
	fmt.Println("Done: ", pointValue)
}

func processLineAtaTime(fractalRegion BoundedArea, windowRegion WindowArea) {
	//var pointValue int = 0

	strideX := (fractalRegion.lowerRight.x - fractalRegion.upperLeft.x) / (float64)(windowRegion.lowerRight.x - windowRegion.upperLeft.x)
	strideY := (fractalRegion.lowerRight.y - fractalRegion.upperLeft.y) / (float64)(windowRegion.lowerRight.y - windowRegion.upperLeft.y)
	fmt.Println("Stride of X:", strideX)
	fmt.Println("Stride of Y:", strideY)

	for y := 0; y < (windowRegion.lowerRight.y - windowRegion.upperLeft.y); y++ {
		xpos := float64(0.0)
		ypos := fractalRegion.upperLeft.y + ((float64)(y) * strideY)

		// Need to build the message that will be passed to the streaming function.
		// The streaming processing handles creating a horizontal line of the Mandelbrot Set
		// It needs to know:
		//		1) where in the complex plane to start
		//		2) how many iterations
		//		3) the distance in the REAL axis between points
		//
		grpcStartPoint := &grpcMandelbrot.ComplexPoint{
			Real:		xpos,
			Imaginary:	ypos,
		}

		grpcStreamDef := &grpcMandelbrot.ComplexStreamDef{
			IterationsForReal: 1920,
			StrideForReal: strideX,
		}

		grpcStreamRequest := &grpcMandelbrot.PointRequestStream{
			Streamdef: grpcStreamDef,
			StartPoint: grpcStartPoint,
		}

		cc, err := grpc.Dial("127.0.0.1:50051", grpc.WithInsecure())
		if err != nil {
			log.Fatalln("Error establishing connection:", err)
		}
	
		mandelbrotClient := grpcMandelbrot.NewComplexPointProcessingServiceClient(cc)
	
		resultStream, err := mandelbrotClient.ProcessPointStream(context.Background(), grpcStreamRequest)
		if err != nil {
			log.Fatalln("Error opening Process Point Stream: ", err)
		}
		for {
			msg, err := resultStream.Recv()
			if err == io.EOF {
				// Server closed the stream
				break
			}
			if err != nil {
				log.Fatalln("Error receiveing Stream data:", err)
			}
			msg.GetResult()
		}
		fmt.Println("Proceesed line:", y)
		cc.Close()
	}
}
