package main

import (
	"fmt"
	"github.com/opentracing/opentracing-go"
	"github.com/uber/jaeger-client-go"
	"github.com/uber/jaeger-client-go/config"
	"golang.org/x/net/context"
	"google.golang.org/grpc"

	//"google.golang.org/grpc/examples/helloworld"
	"google.golang.org/grpc/credentials/insecure"
	"google.golang.org/grpc/grpclog"
	"google.golang.org/grpc/metadata"

	"io"
	//pb "micro/proto/hello"
	"time"
)

const (
	// Address gRPC服务地址
	Address = "127.0.0.1:50052"
)

//grpc提供了拦截器，我们可以在dial函数里设置拦截器，这样每次请求都会经过拦截器，我们不需要在每个接口中去编写重复的代码。
func main() {
	//init jaeger
	jaegerAgentHost := "127.0.0.1:6831"
	tracer, closer, err := initJaeger("client", jaegerAgentHost)
	if err != nil {
		fmt.Println(err)
	}
	defer closer.Close()
	//dial
	conn, err := grpc.Dial(Address, grpc.WithTransportCredentials(insecure.NewCredentials()), clientDialOption(tracer))
	if err != nil {
		fmt.Printf("dial fail, %+v\n\n", err)
	}

	//// 连接
	////conn, err := grpc.Dial(Address, grpc.WithInsecure())
	//conn, err := grpc.Dial(Address, grpc.WithTransportCredentials(insecure.NewCredentials()))
	//if err != nil {
	//    grpclog.Fatalln(err)
	//}
	//defer conn.Close()

	// 初始化客户端
	client := pb.NewHelloClient(conn)
	ctx, cancel := context.WithTimeout(context.TODO(), time.Duration(10)*time.Second)
	defer cancel()
	// 调用方法
	res, err := client.SayHello(ctx, &pb.HelloRequest{Name: "gRPC1212"})

	if err != nil {
		grpclog.Fatalln(err)
	}

	fmt.Println(res.Message)
}

func clientDialOption(tracer opentracing.Tracer) grpc.DialOption {
	return grpc.WithUnaryInterceptor(jaegerGrpcClientInterceptor)
}

type TextMapWriter struct {
	metadata.MD
}

// Set 重写TextMapWriter的Set方法，我们需要将carrier中的数据写入到metadata中，这样grpc才会携带。
func (t TextMapWriter) Set(key, val string) {
	//key = strings.ToLower(key)
	t.MD[key] = append(t.MD[key], val)
}

//span finish之前利用SetTag添加一些额外的信息，例如request和reply，
//以及error信息，但是这些信息是不会在client和server中传递的，我们可以在UI中每个span中显示出他们的tag。
func jaegerGrpcClientInterceptor(ctx context.Context, method string, req, reply interface{}, cc *grpc.ClientConn, invoker grpc.UnaryInvoker, opts ...grpc.CallOption) (err error) {
	var parentContext opentracing.SpanContext
	//先从context中获取原始的span
	parentSpan := opentracing.SpanFromContext(ctx)
	if parentSpan != nil {
		parentContext = parentSpan.Context()
	}
	tracer := opentracing.GlobalTracer()
	span := tracer.StartSpan(method, opentracing.ChildOf(parentContext))
	defer span.Finish()
	//从context中获取metadata。md.(type) == map[string][]string
	md, ok := metadata.FromIncomingContext(ctx)
	if !ok {
		//md = metadata.New(nil)
		m := map[string]string{
			"x-request-id":      "X-Request-Id",
			"x-b3-traceid":      "X-B3-Traceid",
			"x-b3-spanid":       "X-B3-Spanid",
			"x-b3-parentspanid": "X-B3-Parentspanid",
			"x-b3-sampled":      "X-B3-Sampled",
			"x-b3-flags":        "X-B3-Flags",
			"x-ot-span-context": "X-Ot-Span-Context",
		}
		md = metadata.New(m)
	} else {
		//如果对metadata进行修改，那么需要用拷贝的副本进行修改。（FromIncomingContext的注释）
		md = md.Copy()
	}
	//定义一个carrier，下面的Inject注入数据需要用到。carrier.(type) == map[string]string
	//carrier := opentracing.TextMapCarrier{}
	carrier := TextMapWriter{md}
	//将span的context信息注入到carrier中,  相当于序列化
	e := tracer.Inject(span.Context(), opentracing.TextMap, carrier)
	if e != nil {
		fmt.Println("tracer Inject err,", e)
	}
	//创建一个新的context，把metadata附带上
	ctx = metadata.NewOutgoingContext(ctx, md)

	return invoker(ctx, method, req, reply, cc, opts...)
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