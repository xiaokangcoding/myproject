// Package main
package main

import (
	"context"
	"github.com/go-micro/plugins/v4/registry/consul"
	"github.com/opentracing/opentracing-go"
	"github.com/uber/jaeger-client-go"
	"github.com/uber/jaeger-client-go/config"
	"go-micro.dev/v4/registry"
	_ "io"
	"log"
	//hello "github.com/go-micro/examples/greeter/srv/proto/hello"
	"go-micro.dev/v4"
	pb "github.com/Henry-jk/go-microservice-study/greeter/srv/proto"
	//wrapperTrace "github.com/micro/go-plugins/wrapper/trace/opentracing/v2"
	wrapperTrace "github.com/go-micro/plugins/v4/wrapper/trace/opentracing"
)

type Say struct{}

func (s *Say) Hello(ctx context.Context, req *pb.Request, rsp *pb.Response) error {
	//log.Log("Received Say.Hello request")
	rsp.Msg = "Hello " + req.Name
	return nil
}

func main() {

	cfg := &config.Configuration{
		Sampler: &config.SamplerConfig{
			Type:  jaeger.SamplerTypeConst,
			Param: 1,
		},
		Reporter: &config.ReporterConfig{
			LogSpans:           true,
		},
	}

	closer, err := cfg.InitGlobalTracer(
		"my.micro.srv.greeter",
	)
	if err != nil {
		log.Printf("Could not initialize jaeger tracer: %s", err.Error())
		return
	}
	defer closer.Close()

	// 初始化 Consul 注册中心
	consulReg := consul.NewRegistry(
		registry.Addrs("127.0.0.1:8500"), // 替换为您的 Consul 地址
	)

	service := micro.NewService(
		micro.Name("my.micro.srv.greeter"),
		micro.Address("192.168.0.107:9090"),
		micro.Registry(consulReg), // 使用 Consul 注册中心
		//micro.WrapHandler(opentracing.NewHandlerWrapper(opentracing.GlobalTracer())), // Wrap the handler with the Jaeger tracing wrapper
		micro.WrapHandler(wrapperTrace.NewHandlerWrapper(opentracing.GlobalTracer())),
	)

	// optionally setup command line usage
	service.Init()

	// Register Handlers
	pb.RegisterSayHandler(service.Server(), new(Say))

	// Run server
	if err := service.Run(); err != nil {
		log.Fatal(err)
	}
}

//func initJaeger(serviceName string, jaegerAgentHost string) (tracer opentracing.Tracer, closer io.Closer, err error) {
//	//config.FromEnv()
//
//	//tracer, closer, err = cfg.NewTracer()
//	//opentracing.SetGlobalTracer(tracer)
//	return tracer, closer, err
//}
