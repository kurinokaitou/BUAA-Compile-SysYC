@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@.str.1 = private unnamed_addr constant [3 x i8] c"%s\00", align 1

define dso_local i32 @getint() #0 {
  %1 = alloca i32, align 4
  %2 = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i64 0, i64 0), i32* %1)
  %3 = load i32, i32* %1, align 4
  ret i32 %3
}

declare dso_local i32 @__isoc99_scanf(i8*, ...) #1

define dso_local void @putint(i32 %0) #0 {
  %2 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  %3 = load i32, i32* %2, align 4
  %4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i64 0, i64 0), i32 %3)
  ret void
}

declare dso_local i32 @printf(i8*, ...) #1

define dso_local void @putstr(i8* %0) #0 {
  %2 = alloca i8*, align 8
  store i8* %0, i8** %2, align 8
  %3 = load i8*, i8** %2, align 8
  %4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.1, i64 0, i64 0), i8* %3)
  ret void
}

@.str1 = constant [7 x i8] c"sum = \00"
@.str2 = constant [2 x i8] c"\0a\00"

define i32 @add(i32 %a, i32 %b, i32 * %c) #0 {
b0: ; preds = 
	%t0 = alloca i32, align 4
	%x1 = getelementptr inbounds i32, i32* %t0, i32 0
	; store 2
	store i32 %a, i32* %x1, align 4
	%t3 = alloca i32, align 4
	%x4 = getelementptr inbounds i32, i32* %t3, i32 0
	; store 5
	store i32 %b, i32* %x4, align 4
	; getelementptr 6
	%x6 = getelementptr inbounds i32, i32* %c, i32 1
	%x7 = load i32, i32* %x1, align 4
	%x8 = load i32, i32* %x4, align 4
	%x9 = add i32 %x7, %x8
	; store 10
	store i32 %x9, i32* %x6, align 4
	%x11 = load i32, i32* %x1, align 4
	%x12 = load i32, i32* %x4, align 4
	%x13 = add i32 %x11, %x12
	ret i32 %x13
}

define i32 @main() #0 {
b0: ; preds = 
	%t0 = alloca i32, align 4
	%x1 = getelementptr inbounds i32, i32* %t0, i32 0
	; store 2
	store i32 0, i32* %x1, align 4
	%t3 = alloca i32, align 4
	%x4 = getelementptr inbounds i32, i32* %t3, i32 0
	; store 5
	store i32 0, i32* %x4, align 4
	%t6 = alloca [2 x i32], align 4
	%x7 = getelementptr inbounds [2 x i32], [2 x i32]* %t6, i32 0, i32 0
	; store 8
	store i32 0, i32* %x7, align 4
	; store 9
	%t10 = getelementptr inbounds i32, i32* %x7, i32 1
	store i32 0, i32* %t10, align 4
	%x11 = call i32 @getint()
	; store 12
	store i32 %x11, i32* %x1, align 4
	%x13 = call i32 @getint()
	; store 14
	store i32 %x13, i32* %x4, align 4
	%x15 = load i32, i32* %x1, align 4
	%x16 = load i32, i32* %x4, align 4
	; getelementptr 17
	%x17 = getelementptr inbounds i32, i32* %x7, i32 0
	%x18 = call i32 @add(i32 %x15, i32 %x16, i32 * %x17)
	; getelementptr 19
	%x19 = getelementptr inbounds i32, i32* %x7, i32 1
	%x20 = load i32, i32* %x19, align 4
	%t21 = getelementptr inbounds [7 x i8], [7 x i8]* @.str1, i32 0, i32 0
	call void @putstr(i8* %t21)
	call void @putint(i32 %x20)
	%t22 = getelementptr inbounds [2 x i8], [2 x i8]* @.str2, i32 0, i32 0
	call void @putstr(i8* %t22)
	ret i32 0
}

