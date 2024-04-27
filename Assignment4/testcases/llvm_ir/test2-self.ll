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

@"x" = private constant i32 5
define i32 @"main"() 
{
entry:
  %".2" = load i32, i32* @"x"
  call void @"print_int"(i32 %".2")
  %"x-2" = alloca i32
  store i32 10, i32* %"x-2"
  %".5" = load i32, i32* %"x-2"
  call void @"print_int"(i32 %".5")
  %".7" = load i32, i32* %"x-2"
  ret i32 %".7"
}
