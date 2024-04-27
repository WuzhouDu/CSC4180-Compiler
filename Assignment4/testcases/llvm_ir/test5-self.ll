; ModuleID = ""
target triple = "unknown-unknown-unknown"
target datalayout = ""

declare i32* @"array_of_string"(i8* %".1") 

declare i8* @"string_of_array"(i32* %".1") 

declare i32 @"length_of_string"(i8* %".1") 

declare i8* @"string_of_int"(i32 %".1") 

declare i8* @"string_cat"(i8* %".1", i8* %".2") 

declare void @"print_string"(i8* %".1") 

declare void @"print_int"(i32 %".1") 

declare void @"print_bool"(i32 %".1") 

define i32 @"main"() 
{
entry:
  %"value-2" = alloca i32
  store i32 10, i32* %"value-2"
  br label %"loop.header"
loop.header:
  %".4" = load i32, i32* %"value-2"
  %".5" = icmp sgt i32 %".4", 0
  br i1 %".5", label %"loop.body", label %"loop.end"
loop.body:
  %".7" = load i32, i32* %"value-2"
  call void @"print_int"(i32 %".7")
  %".9" = getelementptr [3 x i8], [3 x i8]* @"string_literal_\5cn", i32 0, i32 0
  call void @"print_string"(i8* %".9")
  %".11" = load i32, i32* %"value-2"
  %".12" = sub i32 %".11", 1
  store i32 %".12", i32* %"value-2"
  br label %"loop.header"
loop.end:
  ret i32 0
}

@"string_literal_\5cn" = private constant [3 x i8] c"\5cn\00"