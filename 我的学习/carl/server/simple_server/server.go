package main

import (
	"context"
	"fmt"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
	"log"
	"net"
	"runtime/debug"

	pb "github.com/EDDYCJY/go-grpc-example/proto"
)

type SearchService struct{}

func (s *SearchService) Search(ctx context.Context, r *pb.SearchRequest) (*pb.SearchResponse, error) {
	if ctx.Err() == context.Canceled {
		return nil, status.Errorf(codes.Canceled, "searchService.Search canceled")
	}
	//panic("故意退出。。。。")
	fmt.Println("Search....................")
	return &pb.SearchResponse{Response: r.GetRequest() + " Server"}, nil
}

const PORT = "9001"

func main() {
	//tlsServer := gtls.Server{
	//	CaFile:   "../../conf/ca.pem",
	//	CertFile: "../../conf/server/server.pem",
	//	KeyFile:  "../../conf/server/server.key",
	//}
	//c, err := tlsServer.GetCredentialsByCA()
	//if err != nil {
	//	log.Fatalf("GetTLSCredentialsByCA err: %v", err)
	//}

	//opts := []grpc.ServerOption{
	//	//grpc.Creds(c),
	//	grpc_middleware.WithUnaryServerChain(
	//		RecoveryInterceptor,
	//		aa,
	//		LoggingInterceptor,
	//
	//	),
	//}
	//
	//server := grpc.NewServer(opts...)
	server := grpc.NewServer()
	pb.RegisterSearchServiceServer(server, &SearchService{})

	lis, err := net.Listen("tcp", ":"+PORT)
	if err != nil {
		log.Fatalf("net.Listen err: %v", err)
	}

	server.Serve(lis)
}

func LoggingInterceptor(ctx context.Context, req interface{}, info *grpc.UnaryServerInfo, handler grpc.UnaryHandler) (interface{}, error) {
	log.Printf("gRPC method: %s, %v", info.FullMethod, req)
	resp, err := handler(ctx, req)
	log.Printf("gRPC method: %s, %v", info.FullMethod, req)
	//return nil, nil
	return resp, err
}
func aa(ctx context.Context, req interface{}, info *grpc.UnaryServerInfo, handler grpc.UnaryHandler) (interface{}, error)  {
	fmt.Println("11111111")
	resp, err := handler(ctx, req)
	fmt.Println("22222222")
	return resp, err
//	return nil, nil
}

func RecoveryInterceptor(ctx context.Context, req interface{}, info *grpc.UnaryServerInfo, handler grpc.UnaryHandler) (resp interface{}, err error) {
	defer func() {
		if e := recover(); e != nil {
			fmt.Println("0000000000")
			debug.PrintStack()
			err = status.Errorf(codes.Internal, "Panic err: %v", e)
		}
	}()
	fmt.Println("recover...........")
	return handler(ctx, req)
}
