; Declare printf
declare i32 @printf(i8*, ...)

; Declare scanf
declare i32 @scanf(i8*, ...)

define i32 @main() {
	%A = alloca i32
	store i32 1, i32* %A
	%_tmp_1 = load i32, i32* %A
	%_tmp_2 = add i32 %_tmp_1, 2
	store i32 %_tmp_2, i32* %A
	%_tmp_3 = load i32, i32* %A
	%_tmp_4 = add i32 %_tmp_3, 5
	%_tmp_5 = load i32, i32* %A
	%_tmp_6 = load i32, i32* %A
	%_tmp_7 = load i32, i32* %A
	%_tmp_8 = add i32 %_tmp_7, -1
	%_tmp_9 = add i32 %_tmp_6, %_tmp_8
	%_tmp_10 = add i32 %_tmp_5, %_tmp_9
	%_tmp_11 = add i32 %_tmp_4, %_tmp_10
	%_tmp_12 = add i32 %_tmp_11, 5
	%_tmp_13 = sub i32 %_tmp_12, 1
	%B = alloca i32
	store i32 %_tmp_13, i32* %B
	%C = alloca i32
	store i32 1, i32* %C
	%D = alloca i32
	%_scanf_format_1 = alloca [3 x i8]
	store [3 x i8] c"%d\00", [3 x i8]* %_scanf_format_1
	%_scanf_str_1 = getelementptr [3 x i8], [3 x i8]* %_scanf_format_1, i32 0, i32 0
	call i32 (i8*, ...) @scanf(i8* %_scanf_str_1, i32* %D)
}