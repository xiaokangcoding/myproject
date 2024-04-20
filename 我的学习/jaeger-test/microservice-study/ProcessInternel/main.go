package main

import (
	"context"
	"errors"
	"github.com/opentracing/opentracing-go"
	"github.com/opentracing/opentracing-go/log"
	"github.com/uber/jaeger-client-go"
	jaegercfg "github.com/uber/jaeger-client-go/config"
	"io"
	"time"
)

// initJaeger 将 jaeger tracer 设置为全局 tracer
func initJaeger(service string) io.Closer {
	cfg := jaegercfg.Configuration{
		// 将采样频率设置为 1，每一个 span 都记录，方便查看测试结果
		Sampler: &jaegercfg.SamplerConfig{
			Type:  jaeger.SamplerTypeConst,
			Param: 1,
		},
		Reporter: &jaegercfg.ReporterConfig{
			LogSpans: true,
			// 将 span 发往 jaeger-collector 的服务地址
			CollectorEndpoint: "http://localhost:14268/api/traces",
		},
	}
	closer, err := cfg.InitGlobalTracer(service, jaegercfg.Logger(jaeger.StdLogger))
	if err != nil {
		//panic(fmt.Sprintf("ERROR: cannot init Jaeger: %v\n", err))
	}
	return closer
}

func Bar(ctx context.Context) {
	// 开始一个 span，设置 span 的 operation_name=Bar
	span, ctx := opentracing.StartSpanFromContext(ctx, "Bar")
	defer span.Finish()
	// 模拟执行耗时
	time.Sleep(2 * time.Second)

	// 假设 Bar 发生了某些错误
	err := errors.New("something wrong")
	span.LogFields(
		log.String("event", "error"),
		log.String("message", err.Error()),
	)
	span.SetTag("error", true)
}

func Foo(ctx context.Context) {
	// 开始一个 span, 设置 span 的 operation_name=Foo
	span, ctx := opentracing.StartSpanFromContext(ctx, "Foo")
	defer span.Finish()
	// 将 context 传递给 Bar
	Bar(ctx)
	// 模拟执行耗时
	time.Sleep(1 * time.Second)
}

func main() {
	closer := initJaeger("in-process")
	defer closer.Close()
	// 获取 jaeger tracer
	t := opentracing.GlobalTracer()
	// 创建 root span
	sp := t.StartSpan("in-process-service")
	// main 执行完结束这个 span
	defer sp.Finish()
	// 将 span 传递给 Foo
	ctx := opentracing.ContextWithSpan(context.Background(), sp)
	Foo(ctx)
}