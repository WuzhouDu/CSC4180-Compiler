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
  %"i-3" = alloca i32
  store i32 0, i32* %"i-3"
  %".3" = load i32, i32* %"i-3"
  %".4" = add i32 %".3", 1
  store i32 %".4", i32* %"i-3"
  %".6" = load i32, i32* %"i-3"
  call void @"print_int"(i32 %".6")
  %".8" = getelementptr [3 x i8], [3 x i8]* @"string_literal_\5cn", i32 0, i32 0
  call void @"print_string"(i8* %".8")
  ret i32 0
}

@"string_literal_\5cn" = private constant [3 x i8] c"\5cn\00"