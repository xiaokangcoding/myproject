package microservice_study
////package main
////
//////func main() {
//////	ch := make(chan int)
//////	ch <- 10
//////	fmt.Println("发送成功")
//////}
////
////import (
////	"fmt"
////	"reflect"
////)
////
////func main() {
////	qcrao := Student{age: 18}
////	whatJob(&qcrao)
////
////	growUp(&qcrao)
////	fmt.Println(qcrao)
////
////	stefno := Programmer{age: 100}
////	whatJob(stefno)
////
////	growUp(stefno)
////	fmt.Println(stefno)
////
////	//reflect.Type
////	reflect.ValueOf()
////}
////
////func whatJob(p Person) {
////	p.job()
////}
////
////func growUp(p Person) {
////	p.growUp()
////}
////
////type Person interface {
////	job()
////	growUp()
////}
////
////type Student struct {
////	age int
////}
////
////func (p Student) job() {
////	fmt.Println("I am a student.")
////	return
////}
////
////func (p *Student) growUp() {
////	p.age += 1
////	return
////}
////
////type Programmer struct {
////	age int
////}
////
////func (p Programmer) job() {
////	fmt.Println("I am a programmer.")
////	return
////}
////
////func (p Programmer) growUp() {
////	// 程序员老得太快 ^_^
////	p.age += 10
////	return
////}
//
//
//package main
//
//import (
//	"reflect"
//	"fmt"
//)
//
//type Child struct {
//	Name     string
//	Grade    int
//	Handsome bool
//}
//
//type Adult struct {
//	ID         string `qson:"Name"`
//	Occupation string
//	Handsome   bool
//}
//
//// 如果输入参数 i 是 Slice，元素是结构体，有一个字段名为 `Handsome`，
//// 并且有一个字段的 tag 或者字段名是 `Name` ，
//// 如果该 `Name` 字段的值是 `qcrao`，
//// 就把结构体中名为 `Handsome` 的字段值设置为 true。
//func handsome(i interface{}) {
//	// 获取 i 的反射变量 Value
//	v := reflect.ValueOf(i)
//
//	// 确定 v 是一个 Slice
//	if v.Kind() != reflect.Slice {
//		return
//	}
//
//	// 确定 v 是的元素为结构体
//	if e := v.Type().Elem(); e.Kind() != reflect.Struct {
//		return
//	}
//
//	// 确定结构体的字段名含有 "ID" 或者 json tag 标签为 `name`
//	// 确定结构体的字段名 "Handsome"
//	st := v.Type().Elem()
//
//	// 寻找字段名为 Name 或者 tag 的值为 Name 的字段
//	foundName := false
//	for i := 0; i < st.NumField(); i++ {
//		f := st.Field(i)
//		tag := f.Tag.Get("qson")
//
//		if (tag == "Name" || f.Name == "Name") && f.Type.Kind() == reflect.String {
//			foundName = true
//			break
//		}
//	}
//
//	if !foundName {
//		return
//	}
//
//	if niceField, foundHandsome := st.FieldByName("Handsome"); foundHandsome == false || niceField.Type.Kind() != reflect.Bool {
//		return
//	}
//
//	// 设置名字为 "qcrao" 的对象的 "Handsome" 字段为 true
//	for i := 0; i < v.Len(); i++ {
//		e := v.Index(i)
//		handsome := e.FieldByName("Handsome")
//
//		// 寻找字段名为 Name 或者 tag 的值为 Name 的字段
//		var name reflect.Value
//		for j := 0; j < st.NumField(); j++ {
//			f := st.Field(j)
//			tag := f.Tag.Get("qson")
//
//			if tag == "Name" || f.Name == "Name" {
//				name = v.Index(i).Field(j)
//			}
//		}
//
//		if name.String() == "qcrao" {
//			handsome.SetBool(true)
//		}
//	}
//}
//
//func main() {
//	children := []Child{
//		{Name: "Ava", Grade: 3, Handsome: true},
//		{Name: "qcrao", Grade: 6, Handsome: false},
//	}
//
//	adults := []Adult{
//		{ID: "Steve", Occupation: "Clerk", Handsome: true},
//		{ID: "qcrao", Occupation: "Go Programmer", Handsome: false},
//	}
//
//	fmt.Printf("adults before handsome: %v\n", adults)
//	handsome(adults)
//	fmt.Printf("adults after handsome: %v\n", adults)
//
//	fmt.Println("-------------")
//
//	fmt.Printf("children before handsome: %v\n", children)
//	handsome(children)
//	fmt.Printf("children after handsome: %v\n", children)
//}