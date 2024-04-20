package main

import (
	"context"
	"fmt"
	_ "github.com/go-micro/examples/greeter/srv/proto/hello"
	"github.com/go-micro/plugins/v4/registry/consul"
	"github.com/opentracing/opentracing-go"
	"github.com/uber/jaeger-client-go"
	"github.com/uber/jaeger-client-go/config"
	"go-micro.dev/v4/registry"
	"log"
	"os"
	"os/signal"
	"syscall"

	//hello "github.com/go-micro/examples/greeter/srv/proto/hello"
	"go-micro.dev/v4"
	pb "github.com/Henry-jk/go-microservice-study/greeter/srv/proto"
	wrapperTrace "github.com/go-micro/plugins/v4/wrapper/trace/opentracing"
)

func main() {

	// 创建一个通道，用于接收操作系统信号
	quit := make(chan os.Signal, 1)
	// 通知信号传递到 'quit' 通道
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)

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
		"my.micro.cli.greeter",
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

	// create a new service
	service := micro.NewService(
		micro.Name("my.micro.cli.greeter"),
		micro.Registry(consulReg), // 使用 Consul 注册中心
		micro.WrapClient(wrapperTrace.NewClientWrapper(opentracing.GlobalTracer())),
		)

	// parse command line flags
	service.Init()

	// Use the generated client stub
	cl := pb.NewSayService("my.micro.srv.greeter", service.Client())

	// Make request
	rsp, err := cl.Hello(context.Background(), &pb.Request{
		Name: "John",
	})
	if err != nil {
		fmt.Println(err)
		return
	}

	fmt.Println(rsp.Msg)

	// 阻塞，直到从 'quit' 通道接收到信号
	<-quit

	// 收到信号后执行清理操作
	fmt.Println("Shutting down client...")

	// 退出程序
	fmt.Println("Client gracefully shut down.")
}