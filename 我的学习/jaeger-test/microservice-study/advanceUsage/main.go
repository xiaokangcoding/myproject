package main
//
//import (
//	"log"
//	"context"
//	"strings"
//	"net/http"
//	"encoding/json"
//	"google.golang.org/grpc"
//	"google.golang.org/grpc/metadata"
//	pb "github.com/Henry-jk/jaeger-test/microservice-study
//	opentracing "github.com/opentracing/opentracing-go"
//	"github.com/opentracing/opentracing-go/ext"
//	openLog "github.com/opentracing/opentracing-go/log"
//	"github.com/uber/jaeger-client-go"
//	jaegerCfg "github.com/uber/jaeger-client-go/config"
//)
//
//// metadata 读写
//type MDReaderWriter struct {
//	metadata.MD
//}
//
//// 为了 opentracing.TextMapReader ，参考 opentracing 代码
//func (c MDReaderWriter) ForeachKey(handler func(key, val string) error) error {
//	for k, vs := range c.MD {
//		for _, v := range vs {
//			if err := handler(k, v); err != nil {
//				return err
//			}
//		}
//	}
//	return nil
//}
//
//// 为了 opentracing.TextMapWriter，参考 opentracing 代码
//func (c MDReaderWriter) Set(key, val string) {
//	key = strings.ToLower(key)
//	c.MD[key] = append(c.MD[key], val)
//}
//
//func NewJaegerTracer(serviceName string) (opentracing.Tracer, error) {
//	// 配置项 参考代码 https://github.com/jaegertracing/jaeger-client-go/blob/master/config/config.go
//	cfg := jaegerCfg.Configuration{
//		Sampler: &jaegerCfg.SamplerConfig{
//			Type: "const",
//			Param: 1,
//		},
//		Reporter: &jaegerCfg.ReporterConfig{
//			LogSpans: true,
//			CollectorEndpoint: "http://localhost:14268/api/traces",
//		},
//	}
//
//	cfg.ServiceName = serviceName
//
//	tracer, _, err := cfg.NewTracer(
//		jaegerCfg.Logger(jaeger.StdLogger),
//	)
//
//	if err != nil {
//		log.Println("tracer error ", err)
//	}
//
//	return tracer, err
//}
//
//// 此处参考 grpc文档 https://godoc.org/google.golang.org/grpc#UnaryClientInterceptor
//func interceptor(tracer opentracing.Tracer) grpc.UnaryClientInterceptor{
//	return func (ctx context.Context,
//		method string,
//		req,
//		reply interface{},
//		cc *grpc.ClientConn,
//		invoker grpc.UnaryInvoker,
//		opts ...grpc.CallOption) error {
//		// 创建 rootSpan
//		var rootCtx opentracing.SpanContext
//
//		rootSpan := opentracing.SpanFromContext(ctx)
//		if rootSpan != nil {
//			rootCtx = rootSpan.Context()
//		}
//
//		span := tracer.StartSpan(
//			method,
//			opentracing.ChildOf(rootCtx),
//			opentracing.Tag{"test","hahahahaha"},
//			ext.SpanKindRPCClient,
//		)
//
//		defer span.Finish()
//
//		md, succ := metadata.FromOutgoingContext(ctx)
//		if !succ {
//			md = metadata.New(nil)
//		} else{
//			md = md.Copy()
//		}
//
//		mdWriter := MDReaderWriter{md}
//
//		// 注入 spanContext
//		err := tracer.Inject(span.Context(), opentracing.TextMap, mdWriter)
//
//		if err != nil {
//			span.LogFields(openLog.String("inject error", err.Error()))
//		}
//
//		// new ctx ，并调用后续操作
//		newCtx := metadata.NewOutgoingContext(ctx, md)
//		err = invoker(newCtx, method, req, reply, cc, opts...)
//		if err != nil {
//			span.LogFields(openLog.String("call error", err.Error()))
//		}
//		return err
//	}
//}
//
//// 请求执行的方法
//func hello(w http.ResponseWriter, r *http.Request) {
//	r.ParseForm();
//
//	// new tracer
//	tracer, err := NewJaegerTracer("mainService")
//	if err != nil {
//		log.Fatal("new tracer err ", err)
//	}
//
//	// dial options
//	dialOpts := []grpc.DialOption{grpc.WithInsecure()}
//
//	if tracer != nil {
//		dialOpts = append(dialOpts, grpc.WithUnaryInterceptor(interceptor(tracer)))
//	}
//
//	conn, err := grpc.Dial("localhost:8082", dialOpts...)
//	if err != nil {
//		log.Fatal("connect err ", err)
//	}
//
//	defer conn.Close()
//
//	sv := pb.NewHelloServiceClient(conn)
//
//	var name = "yeshou"
//	if (len(r.Form) > 0 && len(r.Form["name"][0]) > 0) {
//		name = r.Form["name"][0]
//	}
//
//	res, err := sv.SayHello(context.Background(), &pb.HelloReq{Name: name})
//	if err != nil {
//		log.Fatal("c.SayHello func error : ", err)
//	}
//
//	type HelloRes struct{
//		Result    string  `json:"result"`
//	}
//
//	data := HelloRes{
//		Result:  res.Result,
//	}
//
//	jsonData, err := json.Marshal(data)
//	if err != nil {
//		log.Fatal("server error : ", err)
//	}
//
//	w.Write(jsonData)
//}
//
//func main() {
//	http.HandleFunc("/get_h", hello)
//
//	err := http.ListenAndServe(":8081", nil)
//
//	if err != nil {
//		log.Fatal("Listen server err : ", err)
//	}
//
//}