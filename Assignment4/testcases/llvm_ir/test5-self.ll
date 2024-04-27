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
  %".3" = load i32, i32* %"value-2"
  call void @"print_int"(i32 %".3")
  %".5" = getelementptr [2 x i8], [2 x i8]* @"string_literal_\5cn", i32 0, i32 0
  call void @"print_string"(i8* %".5")
  ret i32 0
}

@"string_literal_\5cn" = private constant [2 x i8] c"\5cn", align 1