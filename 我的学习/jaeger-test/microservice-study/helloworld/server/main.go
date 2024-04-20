package main

import (
	_ "fmt"
	pb "github.com/Henry-jk/jaeger-test/microservice-study/helloworld/proto"
	"context"
	_ "fmt"
	_ "github.com/grpc-ecosystem/go-grpc-middleware"
	grpc_opentracing "github.com/grpc-ecosystem/go-grpc-middleware/tracing/opentracing"
	"github.com/opentracing/opentracing-go"
	"github.com/uber/jaeger-client-go"
	"github.com/uber/jaeger-client-go/config"
	"google.golang.org/grpc"
	"google.golang.org/grpc/metadata"
	"log"
	_ "github.com/opentracing/opentracing-go/log"
	"net"
	_ "os"
	"time"
)
type HelloServiceImply struct {
	pb.UnimplementedGreeterServer
}

func Respone(name string) (string,error) {
	resp:=  "Hello " + name
	return resp,nil
}

func (p *HelloServiceImply) SayHello(ctx context.Context,in *pb.HelloRequest)(*pb.HelloReply, error){
	// 埋点
	//span, _ := opentracing.StartSpanFromContext(ctx, "SayHello")
	//defer span.Finish()
	//span.SetTag("Request", in)
	// 实现你的方法
	// TODO
	//resp, err :=Respone(in.Name)
	//if err != nil {
	//	span.SetTag("error", true)
	//	span.LogFields(
	//		Log.String("event", "error"),
	//		Log.String("message", err.Error()),
	//	)
	//}
	resp, _ :=Respone(in.Name)
	response := &pb.HelloReply{Message: resp}

	//span.SetTag("Response", response)
	log.Printf("Received: %v", in.GetName())
	return response, nil
}
/*
func main2(){

	// 初始化 Jaeger tracer
	cfg, _ := config.FromEnv()
	tracer, closer, _ := cfg.NewTracer(config.Logger(jaeger.StdLogger))
	defer closer.Close()
	opentracing.SetGlobalTracer(tracer)

	//////////////////////////////////////
	lis, err := net.Listen("tcp", ":8888")
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	s := grpc.NewServer()
	//pb.RegisterGreeterServer(s, &HelloServiceImply{})
	pb.RegisterGreeterServer(s, new (HelloServiceImply))
	log.Printf("server listening at %v", lis.Addr())
	if err := s.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}
*/
func loggingUnaryServerInterceptor(ctx context.Context, req interface{}, info *grpc.UnaryServerInfo, handler grpc.UnaryHandler) (resp interface{}, err error) {
	md, _ := metadata.FromIncomingContext(ctx)
	log.Printf("metadata: %v", md)
	//fmt.Printf("Metadata received from client: %v\n", md)
	return handler(ctx, req)
}
func myInterceptor(ctx context.Context, req interface{}, info *grpc.UnaryServerInfo, handler grpc.UnaryHandler) (resp interface{}, err error) {
	span, _ := opentracing.StartSpanFromContext(ctx, info.FullMethod)
	defer span.Finish()

	// 在这里添加你需要的标签
	span.SetTag("mykey", "myvalue")
	log.Println("11111111111111111111111111111111111")
	// 然后继续原来的RPC处理
	return handler(ctx, req)
}

func main() {
//	os.Setenv("JAEGER_SERVICE_NAME", "myservice")
	// Jaeger tracer 初始化
	//cfg, err:= config.FromEnv()

	cfg := config.Configuration{
		ServiceName: "mygrpcservice",
		Sampler: &config.SamplerConfig{
			Type:  jaeger.SamplerTypeConst,
			Param: 1,
		},
		Reporter: &config.ReporterConfig{
			LogSpans:            true,
			BufferFlushInterval: 1 * time.Second,
			LocalAgentHostPort:  "localhost:6831",
		},
	}

	//if err != nil {
	//	// 错误处理逻辑
	//	log.Fatal(err)
	//}
	tracer, closer, err := cfg.NewTracer(config.Logger(jaeger.StdLogger))
	if err != nil {
		// 错误处理逻辑
		log.Fatal(err)
	}

	defer closer.Close()
	opentracing.SetGlobalTracer(tracer)

	//s := grpc.NewServer(
	//	grpc.UnaryInterceptor(grpc_opentracing.UnaryServerInterceptor()),
	//	//grpc.UnaryInterceptor(loggingUnaryServerInterceptor),
	//)
	//
	s := grpc.NewServer(
		grpc.ChainUnaryInterceptor(
			//myInterceptor,
			grpc_opentracing.UnaryServerInterceptor(),
			loggingUnaryServerInterceptor,
		),
	)


	pb.RegisterGreeterServer(s, new (HelloServiceImply))

	lis, err := net.Listen("tcp", ":8888")
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	if err := s.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}






///////////////////////////////////////////////////////////////////////////////////////////






