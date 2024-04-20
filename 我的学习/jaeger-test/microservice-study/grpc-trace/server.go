package main

import "github.com/opentracing/opentracing-go"

import (
	"context"
	"fmt"

	"github.com/uber/jaeger-client-go"
	"github.com/uber/jaeger-client-go/config"
	"google.golang.org/grpc"
	"google.golang.org/grpc/grpclog"
	"google.golang.org/grpc/metadata"
	"io"

	pb "micro/proto/hello" // 引入proto包
	"net"
)

const (
	// Address gRPC服务地址
	Address = "127.0.0.1:50052"
)

// 定义helloService并实现约定的接口
type helloService struct{}

// HelloService Hello服务
var HelloService = helloService{}

// SayHello 实现Hello服务接口
func (h helloService) SayHello(ctx context.Context, in *pb.HelloRequest) (*pb.HelloResponse, error) {
	resp := new(pb.HelloResponse)
	resp.Message = fmt.Sprintf("Hello %s.", in.Name)
	return resp, nil
}

//我们使用Extract函数将carrier从metadata中提取出来，
//这样client端与server端就能建立span信息的关联。我们在server端同样只是修改拦截器，
//在grpc.NewServer时将我们的拦截器传进去。
func main() {
	jaegerAgentHost := "127.0.0.1:6831"
	tracer, closer, err := initJaeger("client", jaegerAgentHost)
	if err != nil {
		fmt.Println(err)
	}
	defer closer.Close()
	var serviceOpts []grpc.ServerOption

	listen, err := net.Listen("tcp", Address)
	if err != nil {
		grpclog.Fatalf("Failed to listen: %v", err)
	}

	if tracer != nil {
		serviceOpts = append(serviceOpts, serverOption(tracer))
	}

	// 实例化grpc Server
	s := grpc.NewServer(serviceOpts...)

	// 注册HelloService
	pb.RegisterHelloServer(s, HelloService)

	grpclog.Println("Listen on " + Address)
	s.Serve(listen)
}

func serverOption(tracer opentracing.Tracer) grpc.ServerOption {
	return grpc.UnaryInterceptor(jaegerGrpcServerInterceptor)
}

type TextMapReader struct {
	metadata.MD
}

// ForeachKey 读取metadata中的span信息
func (t TextMapReader) ForeachKey(handler func(key, val string) error) error { //不能是指针
	for key, val := range t.MD {
		for _, v := range val {
			if err := handler(key, v); err != nil {
				return err
			}
		}
	}
	return nil
}

func jaegerGrpcServerInterceptor(ctx context.Context, req interface{}, info *grpc.UnaryServerInfo, handler grpc.UnaryHandler) (resp interface{}, err error) {
	//从context中获取metadata。md.(type) == map[string][]string
	md, ok := metadata.FromIncomingContext(ctx)
	if !ok {
		md = metadata.New(nil)
	} else {
		//如果对metadata进行修改，那么需要用拷贝的副本进行修改。（FromIncomingContext的注释）
		md = md.Copy()
	}
	fmt.Println(md)
	carrier := TextMapReader{md}
	tracer := opentracing.GlobalTracer()
	spanContext, e := tracer.Extract(opentracing.TextMap, carrier)
	if e != nil {
		fmt.Println("Extract err:", e)
	}

	span := tracer.StartSpan(info.FullMethod, opentracing.ChildOf(spanContext))
	defer span.Finish()
	ctx = opentracing.ContextWithSpan(ctx, span)

	return handler(ctx, req)
}

func initJaeger(serviceName string, jaegerAgentHost string) (tracer opentracing.Tracer, closer io.Closer, err error) {
	cfg := &config.Configuration{
		Sampler: &config.SamplerConfig{
			Type:  "const",
			Param: 1,
		},
		Reporter: &config.ReporterConfig{
			LogSpans:           true,
			LocalAgentHostPort: jaegerAgentHost,
		},
		ServiceName: serviceName,
	}
	tracer, closer, err = cfg.NewTracer(config.Logger(jaeger.StdLogger))
	opentracing.SetGlobalTracer(tracer)
	return tracer, closer, err
}
