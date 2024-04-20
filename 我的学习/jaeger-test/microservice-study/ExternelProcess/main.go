package main

import (
	"fmt"
	"net/http"

	"os"
	"time"

	"github.com/opentracing/opentracing-go"
	"github.com/uber/jaeger-client-go"
	jaegercfg "github.com/uber/jaeger-client-go/config"
	"github.com/uber/jaeger-lib/metrics"
)

func main() {
	cfg := &jaegercfg.Configuration{
		ServiceName: "GoService",
		Sampler: &jaegercfg.SamplerConfig{
			Type:  jaeger.SamplerTypeConst,
			Param: 1,
		},
		Reporter: &jaegercfg.ReporterConfig{
			LogSpans:           true,
			BufferFlushInterval: 1 * time.Second,
		},
	}

	jMetricsFactory := metrics.NullFactory
	tracer, closer, err := cfg.NewTracer(
		jaegercfg.Metrics(jMetricsFactory),
	)
	if err != nil {
		fmt.Printf("Could not initialize jaeger tracer: %s\n", err.Error())
		os.Exit(1)
	}
	defer closer.Close()
	opentracing.SetGlobalTracer(tracer)

	http.HandleFunc("/goservice", func(w http.ResponseWriter, r *http.Request) {
		span := tracer.StartSpan("/goservice")
		defer span.Finish()

		// Add your service logic here

		w.Write([]byte("Hello from Go Service"))
	})

	http.ListenAndServe(":8080", nil)
}
