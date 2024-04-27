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
  %"y-2" = alloca i32
  store i32 5, i32* %"y-2"
  %".3" = load i32, i32* %"y-2"
  call void @"print_int"(i32 %".3")
  %".5" = getelementptr [3 x i8], [3 x i8]* @"string_literal_\5cn", i32 0, i32 0
  call void @"print_string"(i8* %".5")
  %".7" = load i32, i32* %"y-2"
  %".8" = icmp sgt i32 %".7", 0
  br i1 %".8", label %"if_block", label %"else_block"
if_block:
  %"is_y_positive-3" = alloca i32
  store i32 1, i32* %"is_y_positive-3"
  %".11" = load i32, i32* %"is_y_positive-3"
  call void @"print_bool"(i32 %".11")
  br label %"merge_block"
else_block:
  %"is_y_positive-4" = alloca i32
  store i32 0, i32* %"is_y_positive-4"
  %".15" = load i32, i32* %"is_y_positive-4"
  call void @"print_bool"(i32 %".15")
  br label %"merge_block"
merge_block:
  ret i32 0
}

@"string_literal_\5cn" = private constant [3 x i8] c"\5cn\00"