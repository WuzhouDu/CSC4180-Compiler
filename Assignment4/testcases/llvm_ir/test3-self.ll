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
  %"y" = alloca i32
  store i32 5, i32* %"y"
  %"is_y_positive" = alloca i32
  store i32 1, i32* %"is_y_positive"
  %"is_y_positive.1" = alloca i32
  store i32 0, i32* %"is_y_positive.1"
}
