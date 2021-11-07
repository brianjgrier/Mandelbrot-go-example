package main

import (
	"context"
	"net"

	"github.com/sirupsen/logrus"
	"google.golang.org/grpc"

	grpcMandelbrot "github.com/brianjgrier/Mandelbrot-go-example/pkg/generated"
)

// just a comment
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

func getResult(realPart float64, imanginaryPart float64) int32 {
	var result, i int32 = -1, 0
	var x1, y1 float64

	c := complex(realPart, imanginaryPart)
	z := complex(0, 0)

	for i = 0; i < 1000; i++ {
		result = i
		z = (z * z) + c
		x1 = real(z)
		y1 = imag(z)

		if (x1*x1 + y1*y1) > 4 {
			break
		}
	}
	return result
}

type mandelServer struct{}

func init() {
	logrus.SetFormatter(&logrus.JSONFormatter{
		PrettyPrint: true,
	})
}

func (s *mandelServer) ProcessPoint(ctx context.Context, in *grpcMandelbrot.PointRequest) (*grpcMandelbrot.PointResult, error) {

	result := getResult(in.GetThePoint().GetReal(), in.GetThePoint().GetImaginary())
	//logrus.Info("Result = ", result)

	return &grpcMandelbrot.PointResult{Result: result}, nil
}

func (s *mandelServer) ProcessPointStream(in *grpcMandelbrot.PointRequestStream, stream grpcMandelbrot.ComplexPointProcessingService_ProcessPointStreamServer) error {
	var result, loop int32 = -1, 0

	iterations := in.GetStreamdef().GetIterationsForReal()
	realStride := in.GetStreamdef().GetStrideForReal()

	if iterations <= 0 {
		logrus.Error("Received stream request with invalid iterator %v", iterations)
	} else {
		realValue := in.GetStartPoint().GetReal()
		imagineValue := in.GetStartPoint().GetImaginary()
		for loop = 0; loop < iterations; loop++ {
			result = getResult(realValue, imagineValue)
			res := &grpcMandelbrot.PointStreamResult{
				Result: result,
			}
			stream.Send(res)
			realValue += realStride
		}
	}
	return nil
}

func main() {
	serverAdr := "localhost:8083"
	listenAddr, err := net.Listen("tcp", serverAdr)
	if err != nil {
		logrus.Fatalf("Error while starting the listening service %v", err.Error())
	}
	defer listenAddr.Close()

	grpcServer := grpc.NewServer()

	grpcMandelbrot.RegisterComplexPointProcessingServiceServer(grpcServer, &mandelServer{})

	logrus.Infof("Server starting to listen on %s", serverAdr)
	if err = grpcServer.Serve(listenAddr); err != nil {
		logrus.Fatalf("Error while starting the gRPC server on the %s listen address %v", listenAddr, err.Error())
	}
	//
}
